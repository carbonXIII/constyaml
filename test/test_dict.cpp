#include <gtest/gtest.h>

#include <constyaml/types.h>
#include <constyaml/dict.h>

using namespace constyaml;
using namespace constyaml::literals;
using namespace std::literals;

TEST(ConstDict, IsStructural) {
  static constexpr dict dict = {
    std::pair {string_literal{"foo"}, string_literal{"bar"}},
    std::pair {string_literal{"apple"}, 420},
    std::pair {string_literal{"orange"}, 69}
  };

  static_assert(get<dict>("foo"_k) == "bar"sv);
  static_assert(get<dict>("apple"_k) == 420);
  static_assert(get<dict>("orange"_k) == 69);
}

