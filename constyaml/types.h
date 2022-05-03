#pragma once

#include <algorithm>
#include <array>
#include <string_view>

namespace constyaml {
  template <typename... T>
  struct pack {};

  template <typename T>
  struct tag {
    using type = T;
  };

  template<size_t N>
  struct string_literal {
    constexpr string_literal(const char (&str)[N]) {
      std::copy_n(str, N, value);
    }

    constexpr string_literal(const std::string_view s) {
      std::copy(s.begin(), s.end(), value);
      value[N-1] = '\0';
    }

    constexpr bool operator==(const std::string_view s) const {
      if(s.size() != N-1) return false;
      for(int i = 0; i < N-1; i++)
        if(value[i] != s[i])
          return false;
      return true;
    }

    constexpr char operator[](int i) const {
      return value[i];
    }

    constexpr operator const std::string_view() const {
      return std::string_view(value, N-1);
    }

    friend std::ostream& operator<<(std::ostream& os, const string_literal& s) {
      return (os << s.value);
    }

    static constexpr size_t len = N;
    char value[N];
  };

  template <typename T, int Cap>
  struct big_array: std::array<T, Cap> {
    std::size_t actual_len = 0;

    constexpr void push_back(const T& v) {
      this->operator[](actual_len++) = v;
    }

    constexpr void pop_back() {
      --actual_len;
    }

    constexpr auto& back() {
      return this->operator[](actual_len - 1);
    }

    constexpr std::size_t size() const { return actual_len; }

    constexpr auto end() const { return this->begin() + size(); }
  };
}
