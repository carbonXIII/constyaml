#pragma once

#include <array>
#include <vector>
#include <optional>
#include <tuple>
#include <string_view>

#include "types.h"
#include "dict.h"
#include "list.h"

namespace constyaml::yaml {
  namespace detail {
    void parser_error(auto) {}

    constexpr bool is_space(char c) { return c == ' ' || c == '\n'; }
    constexpr bool is_quote(char c) { return c == '\'' || c == '"'; }

    template <typename T, int n>
    struct identity {
      using type = T;
    };

    template <int idx, int... mask>
    constexpr int get_mask_next() {
      int sum = 0;
      return ((sum += mask, sum < idx + 1) + ...);
    }

    enum struct Type {
      NONE, IDT, DDT, SCALAR, DASH, COLON, MAP, LIST, OBJ
    };

    struct Token {
      int s, t;
      Type type;
    };

    struct Tree {
      int s = 0, t = 0;
      Type type = Type::NONE;
      std::vector<Tree> kids;

      template <Type type>
      static constexpr auto merge(Tree&& a, Tree&& b) {
        auto ret = Tree{a.s, b.t, type};
        ret.kids = std::move(a.kids);
        ret.kids.reserve(ret.kids.size() + b.kids.size());
        for(auto& kid: b.kids) ret.kids.emplace_back(std::move(kid));
        return ret;
      }

      template <Type type, int... mask>
      static constexpr auto from(typename identity<Tree, mask>::type&&... v) {
        constexpr int M = (mask + ...);

        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          constexpr int N = sizeof...(v);
          auto V = std::tie(v...);

          auto ret = Tree{std::get<0>(V).s, std::get<N-1>(V).t, type};
          ret.kids = { std::forward<Tree>(std::get<get_mask_next<I, mask...>()>(V))... };
          return ret;
        }(std::make_index_sequence<M>{});
      }

      template <Type type, int... mask>
      static constexpr auto from_with_empty(typename identity<Tree, mask>::type&&... v) {
        auto ret = from<type, mask...>(std::forward<decltype(v)>(v)...);
        ret.kids.emplace_back();
        return ret;
      }
    };

    /// Get a list of tokens from the input text
    constexpr auto tokenize(const std::string_view body) {
      struct Line { int s, t, col; };

      std::vector<Line> lines;
      int s = 0, col = 0, first_char = 0;
      char quoted = 0;
      for(int i = 0; i < body.size(); i++) {
        if(quoted) {
          if(body[i] == quoted && body[i-1] != '\\') {
            quoted = 0;
          }
        }

        else if(body[i] == ' ' && !first_char) col++;

        else if(body[i] == '\n') {
          if(first_char) lines.push_back({s, i, col});
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
        tokens.push_back({(int)body.size(), (int)body.size(), Type::DDT});
        col_stack.pop_back();
      }

      return tokens;
    }

    /// Turn a list of tokens into a dynamic syntax tree
    constexpr auto build_tree(const std::vector<Token>& tokens) {
      std::vector<Tree> stack;

      auto rule = [&](const auto& f, auto... types) -> bool {
        constexpr int depth = sizeof...(types);
        if(stack.size() < depth) return false;

        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          bool good = ((stack[stack.size() - depth + I].type == types) && ...);
          if(!good) return false;

          Tree t = f(std::move(stack[stack.size() - depth + I])...);
          for(int z = depth; z > 0; z--) stack.pop_back();
          stack.emplace_back(std::move(t));

          return true;
        }(std::make_index_sequence<depth>{});
      };

      for(auto& token: tokens) {
        stack.emplace_back(token.s, token.t, token.type);

        bool flag = true;
        while(flag) {
          flag = false;

          flag |= rule(Tree::from<Type::OBJ, 1, 0>, Type::SCALAR, Type::DDT);
          flag |= rule(Tree::from<Type::OBJ, 1, 0>, Type::LIST, Type::DDT);
          flag |= rule(Tree::from<Type::OBJ, 1, 0>, Type::MAP, Type::DDT);

          flag |= rule(Tree::from<Type::MAP, 1, 0, 1>, Type::SCALAR, Type::COLON, Type::OBJ);
          flag |= rule(Tree::from<Type::MAP, 1, 0, 0, 1>, Type::SCALAR, Type::COLON, Type::IDT, Type::OBJ);
          flag |= rule(Tree::from_with_empty<Type::MAP, 1, 0, 0>, Type::SCALAR, Type::COLON, Type::DDT);
          flag |= rule(Tree::merge<Type::MAP>, Type::MAP, Type::MAP);

          flag |= rule(Tree::from<Type::LIST, 0, 1>, Type::DASH, Type::OBJ);
          flag |= rule(Tree::from_with_empty<Type::LIST, 0, 0>, Type::DASH, Type::DDT);
          flag |= rule(Tree::merge<Type::LIST>, Type::LIST, Type::LIST);
        }
      }

      if(stack.size() > 1)
        detail::parser_error("Something bad happened while building the parse tree. Perhaps the syntax is invalid.");

      return stack[0];
    }

    /// Flatten a syntax tree so that it is structural and can be used to build a static object
    constexpr auto flatten_tree(const Tree& tree) {
      big_array<Token, 1024> flat;
      std::vector<const Tree*> q;

      q.push_back(&tree);

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

    static constexpr auto parse_flat(const std::string_view body) {
      auto tokens = tokenize(body);
      auto tree = build_tree(tokens);
      return flatten_tree(tree);
    }

    template <Type type>
    struct Unknown {};

    /// Expand a flattened tree into a static object
    template <big_array flat, int node>
    static constexpr auto parse_unfold(const std::string_view body) {
      if constexpr(flat[node].type == Type::SCALAR) {
        const std::string_view s = body.substr(flat[node].s, flat[node].t - flat[node].s);
        return string_literal<flat[node].t - flat[node].s + 1>(s);
      }

      else if constexpr(flat[node].type == Type::MAP) {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          return dict {
            std::pair {
              parse_unfold<flat, flat[node].s+I*2>(body),
              parse_unfold<flat, flat[node].s+I*2+1>(body)
            }...
          };
        }(std::make_index_sequence<(flat[node].t - flat[node].s) / 2>{});
      }

      else if constexpr(flat[node].type == Type::LIST) {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          return list {
            parse_unfold<flat, flat[node].s+I>(body)...
          };
        }(std::make_index_sequence<flat[node].t - flat[node].s>{});
      }

      else if constexpr(flat[node].type == Type::OBJ) {
        return parse_unfold<flat, flat[node].s>(body);
      }

      else if constexpr(flat[node].type == Type::NONE) {
        return std::nullopt;
      }

      else {
        return Unknown<flat[node].type>{};
      }
    }
  }

  static constexpr auto parse(auto callable) {
    constexpr auto body = callable();
    constexpr auto flat = detail::parse_flat(body);
    return detail::parse_unfold<flat, 0>(body);
  }
}

namespace constyaml::literals {
  template <string_literal s> constexpr auto operator "" _yaml() {
    return yaml::parse([](){ return (std::string_view)s; });
  }
}
