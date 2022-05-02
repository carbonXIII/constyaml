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
  using namespace constyaml::literals;

  static constexpr auto A = yaml::parse([]() { return "test\n"sv; });
  static_assert(A == "test"sv);

  static constexpr auto B = yaml::parse([]() { return "foo: bar\napple: 420\norange: 69\n"sv; });
  static_assert(get<B>("foo"_k) == "bar"sv);
  static_assert(get<B>("apple"_k) == "420");
  static_assert(get<B>("orange"_k) == "69");

  static constexpr auto C = yaml::parse([]() { return "- foo\n- bar\n- 50\n- 5.1\n"sv; });
  static_assert(get<0>(C) == "foo"sv);
  static_assert(get<1>(C) == "bar"sv);
  static_assert(get<2>(C) == "50");
  static_assert(get<3>(C) == "5.1");

  static constexpr auto D = yaml::parse([]() {return "a:\n  x: 99\n  y:\n  - 4\n  - 5\n  - 6\nb: 2\nc: 3\n"sv;});
  static_assert(get<D>("a"_k / "x"_k) == "99"sv);
  static_assert(get<D>("a"_k / "y"_k).N == 3);
  static_assert(get<D>("b"_k) == "2"sv);
  static_assert(get<D>("c"_k) == "3"sv);
}
