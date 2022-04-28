#include <iostream>
#include <string_view>

#include <constyaml/const_dict.h>
#include <constyaml/yaml.h>

using namespace constyaml;
using namespace std::literals;

void parser_error(auto) {}

constexpr int parse_unsigned(const std::string_view s) {
  std::size_t ret = 0;
  for(char x: s) {
    if(x < '0' || x >= '9') parser_error("invalid character in unsigned integer");
    ret = ret * 10 + (x - '0');
  }
  return ret;
}

constexpr int parse_true_false(const std::string_view s) {
  if(s == "true") return true;
  if(s != "false") parser_error("expected true or false");
  return false;
}

template <const_dict dict>
struct config_impl {
#define REQUIRE(x,y) static auto x() {                                  \
    static constexpr auto v = get<dict>([]() { return #x##sv; });       \
    static constexpr auto v2 = y(v);                                    \
    return v2;                                                          \
  }

  REQUIRE(apple, parse_unsigned);
  REQUIRE(banana, parse_true_false);
  REQUIRE(orange, std::string_view);

#undef REQUIRE
};

using config = config_impl<yaml::parse([]() { return "apple: 714\nbanana: true\norange: 2\n"sv; })>;

int main() {
  std::cout << config::apple() << std::endl;
  std::cout << config::banana() << std::endl;
  std::cout << config::orange() << std::endl;

  return 0;
}
