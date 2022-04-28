#include <gtest/gtest.h>

#include <constyaml/types.h>
#include <constyaml/dict.h>

using namespace constyaml;

TEST(ConstDict, IsStructural) {
  static constexpr dict dict = {
    std::pair {string_literal{"foo"}, string_literal{"bar"}},
    std::pair {string_literal{"apple"}, 420},
    std::pair {string_literal{"orange"}, 69}
  };

  {
    using namespace std::literals;

    static_assert(get<dict>([]() { return "foo"sv; }) == "bar"sv);
    static_assert(get<dict>([]() { return "apple"sv; }) == 420);
    static_assert(get<dict>([]() { return "orange"sv; }) == 69);
  }
}

