#include <gtest/gtest.h>

#include <constyaml/yaml.h>

using namespace constyaml;

TEST(Yaml, YamlParsingFlat) {
  using namespace std::literals;

  static constexpr auto A = yaml::detail::parse_flat("a:\n  x: 99\n  y:\n  - 4\n  - 5\n  - 6\nb: 2\nc: 3\n"sv);

  static constexpr auto B = yaml::detail::parse_flat("name: Ford Prefect\nage: 42\npossessions:\n- Towel\n"sv);
}

TEST(Yaml, YamlParsingUnfolded) {
  using namespace std::literals;

  static constexpr auto A = yaml::parse([]() { return "test\n"sv; });
  static_assert(A == "test"sv);

  static constexpr auto B = yaml::parse([]() { return "foo: bar\napple: 420\norange: 69\n"sv; });
  static_assert(get<B>([]() { return "foo"sv; }) == "bar"sv);
  static_assert(get<B>([]() { return "apple"sv; }) == "420");
  static_assert(get<B>([]() { return "orange"sv; }) == "69");

  static constexpr auto C = yaml::parse([]() { return "- foo\n- bar\n- 50\n- 5.1\n"sv; });
  static_assert(get<0>(C) == "foo"sv);
  static_assert(get<1>(C) == "bar"sv);
  static_assert(get<2>(C) == "50");
  static_assert(get<3>(C) == "5.1");

  static constexpr auto D = yaml::parse([]() {return "a:\n  x: 99\n  y:\n  - 4\n  - 5\n  - 6\nb: 2\nc: 3\n"sv;});
  // TODO: test the components of this object once the syntax is improved
}
