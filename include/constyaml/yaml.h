#pragma once

#include <array>
#include <vector>
#include <optional>

#include "types.h"
#include "dict.h"
#include "list.h"

namespace constyaml::yaml {
  namespace detail {
    void parser_error(auto) {}

    constexpr bool is_space(char c) { return c == ' ' || c == '\n'; }
    constexpr bool is_quote(char c) { return c == '\'' || c == '"'; }
  }

  enum struct Type {
    NONE, IDT, DDT, SCALAR, DASH, COLON, MAP, LIST, OBJ
  };

  struct Token {
    int s, t;
    Type type;
  };

  constexpr auto parse_flat(auto callable) {
    constexpr auto body = callable();

    // parse lines
    struct Line {
      int s, t, col;
    };

    std::vector<Line> lines;
    int s = 0, col = 0, first_char = 0;
    char quoted = 0;
    for(int i = 0; i < body.size(); i++) {
      if(quoted) {
        if(body[i] == quoted) {
          quoted = 0;
        }
      }

      else if(body[i] == ' ' && !first_char) col++;

      else if(body[i] == '\n') {
        lines.push_back({s, i, col});
        s = i;
        col = first_char = 0;
      }

      else {
        if(first_char == 0 && body[i] == '-')
          col++;

        first_char = 1;
        if(detail::is_quote(body[i])) {
          quoted = body[i];
        }
      }
    }

    // tokenize
    std::vector<Token> tokens;
    std::vector<int> col_stack;

    for(std::size_t i = 0; i < lines.size(); i++) {
      auto& line = lines[i];

      if(col_stack.size() && col_stack.back() >= line.col) {
        while(col_stack.size() && col_stack.back() >= line.col) {
          col_stack.pop_back();
          tokens.push_back({line.s, line.s, Type::DDT});
        }

        col_stack.push_back(line.col);
      } else {
        if(col_stack.size())
          tokens.push_back({line.s, line.s, Type::IDT});
        col_stack.push_back(line.col);
      }

      auto get = [&](int x) {
        if(x < line.t - line.s) return body[line.s + x];
        return '\n';
      };

      int m = line.t - line.s;
      for(int j = 0; j < m; j++) {
        if(get(j) == ':' && detail::is_space(get(j+1))) {
          tokens.push_back({line.s + j, line.s + j + 1, Type::COLON});
          ++j;
        }

        else if(get(j) == '-' && detail::is_space(get(j+1))) {
          tokens.push_back({line.s + j, line.s + j + 1, Type::DASH});
          ++j;
        }

        else if(detail::is_quote(get(j))) {
          int k = j+1;
          for(; k < m && get(j) != get(k); k++);

          tokens.push_back({line.s + j + 1, line.s + k, Type::SCALAR});
          j = k;
        }

        else if(tokens.size() && tokens.back().type == Type::SCALAR) {
          tokens.back().t = line.s + j + 1;
        }

        // TODO: handle merging multi-line strings

        else if(!detail::is_space(get(j))) {
          tokens.push_back({line.s + j, line.s + j + 1, Type::SCALAR});
        }
      }
    }

    while(col_stack.size()) {
      tokens.push_back({body.size(), body.size(), Type::DDT});
      col_stack.pop_back();
    }

    // build syntax tree
    struct Tree {
      int s = 0, t = 0;
      Type type = Type::NONE;
      std::vector<Tree> kids;
    };

    auto tree_merge = [](Type type, Tree&& a, Tree&& b) {
      auto ret = Tree{a.s, b.t, type};
      ret.kids = std::move(a.kids);
      ret.kids.reserve(ret.kids.size() + b.kids.size());
      for(auto& kid: b.kids) ret.kids.emplace_back(std::move(kid));
      return ret;
    };

    std::vector<Tree> stack;

    auto rule = [&](const auto& f, auto... types) -> bool {
      constexpr int depth = sizeof...(types);
      if(stack.size() < depth) return false;

      int z = stack.size() - depth;
      bool good = ((stack[z++].type == types) && ...);
      if(!good) return false;

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        Tree t = f(std::move(stack[stack.size() - depth + I])...);
        for(int z = depth; z > 0; z--) stack.pop_back();
        stack.emplace_back(std::move(t));
      }(std::make_index_sequence<depth>{});

      return true;
    };

    for(auto& token: tokens) {
      stack.emplace_back(token.s, token.t, token.type);

      bool flag = true;
      while(flag) {
        flag = false;

        // TODO: there's probably a better way to do this
        flag |= rule([](Tree&& a, Tree&& b) { return Tree{a.s, b.t, Type::OBJ, {std::forward<Tree>(a)}}; }, Type::SCALAR, Type::DDT);
        flag |= rule([](Tree&& a, Tree&& b) { return Tree{a.s, b.t, Type::OBJ, {std::forward<Tree>(a)}}; }, Type::LIST, Type::DDT);
        flag |= rule([](Tree&& a, Tree&& b) { return Tree{a.s, b.t, Type::OBJ, {std::forward<Tree>(a)}}; }, Type::MAP, Type::DDT);

        flag |= rule([](Tree&& a, Tree&&, Tree&& b) { return Tree {a.s, b.t, Type::MAP, {std::forward<Tree>(a), std::forward<Tree>(b)}}; }, Type::SCALAR, Type::COLON, Type::OBJ);
        flag |= rule([](Tree&& a, Tree&&, Tree&&, Tree&& b) { return Tree {a.s, b.t, Type::MAP, {std::forward<Tree>(a), std::forward<Tree>(b)}}; }, Type::SCALAR, Type::COLON, Type::IDT, Type::OBJ);
        flag |= rule([](Tree&& a, Tree&&, Tree&& b) { return Tree {a.s, b.t, Type::MAP, {std::forward<Tree>(a), Tree()}}; }, Type::SCALAR, Type::COLON, Type::DDT);
        flag |= rule([&](Tree&& a, Tree&& b) { return tree_merge(Type::MAP, std::forward<Tree>(a), std::forward<Tree>(b)); }, Type::MAP, Type::MAP);

        flag |= rule([](Tree&& a, Tree&& b) { return Tree{a.s, b.t, Type::LIST, {std::forward<Tree>(b)}}; }, Type::DASH, Type::OBJ);
        flag |= rule([&](Tree&& a, Tree&& b) { return Tree{a.s, b.t, Type::LIST, {Tree()}}; }, Type::DASH, Type::DDT);
        flag |= rule([&](Tree&& a, Tree&& b) { return tree_merge(Type::LIST, std::forward<Tree>(a), std::forward<Tree>(b)); }, Type::LIST, Type::LIST);
      }
    }

    if(stack.size() > 1) detail::parser_error("Something bad happened while building the parse tree. Perhaps the syntax is invalid.");

    // flatten the syntax tree in BFS order
    big_array<Token, 1024> flat;
    std::vector<Tree*> q;

    for(auto& tree: stack) {
      q.push_back(&tree);
    }

    for(int i = 0; i < q.size(); i++) {
      int lo = q.size();
      for(auto& kid: q[i]->kids) {
        q.push_back(&kid);
      }
      int hi = q.size();

      // terminals point to text
      if(lo == hi) {
        lo = q[i]->s;
        hi = q[i]->t;
      }

      flat.push_back(Token{lo, hi, q[i]->type});
    }

    return flat;
  }

  template <Type type>
  struct Unknown {};

  template <big_array flat, int node>
  static constexpr auto parse_unfold(auto get_substr) {
    if constexpr(flat[node].type == Type::SCALAR) {
      return string_literal<flat[node].t - flat[node].s + 1>(get_substr(flat[node].s, flat[node].t));
    }

    else if constexpr(flat[node].type == Type::MAP) {
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return dict {
          std::pair {
            parse_unfold<flat, flat[node].s+I*2>(get_substr),
            parse_unfold<flat, flat[node].s+I*2+1>(get_substr)
          }...
        };
      }(std::make_index_sequence<(flat[node].t - flat[node].s) / 2>{});
    }

    else if constexpr(flat[node].type == Type::LIST) {
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return list {
          parse_unfold<flat, flat[node].s+I>(get_substr)...
        };
      }(std::make_index_sequence<flat[node].t - flat[node].s>{});
    }

    else if constexpr(flat[node].type == Type::OBJ) {
      return parse_unfold<flat, flat[node].s>(get_substr);
    }

    else if constexpr(flat[node].type == Type::NONE) {
      return std::nullopt;
    }

    else {
      return Unknown<flat[node].type>{};
    }
  }

  static constexpr auto parse(auto callable) {
    constexpr auto body = callable();
    constexpr auto flat = parse_flat(callable);
    return parse_unfold<flat, 0>([&](int s, int t) { return body.substr(s, t - s); });
  }
}
