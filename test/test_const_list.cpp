#include <gtest/gtest.h>

#include <constyaml/types.h>
#include <constyaml/const_list.h>

using namespace constyaml;

TEST(ConstList, IsStructural) {
  using namespace std::literals;

  static constexpr const_list list = {
    string_literal{"foo"},
    string_literal{"bar"},
    50,
    5.1
  };

  static_assert(get<0>(list) == "foo"sv);
  static_assert(get<1>(list) == "bar"sv);
  static_assert(get<2>(list) == 50);
  static_assert(get<3>(list) == 5.1);
}
