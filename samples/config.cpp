#include <iostream>
#include <string_view>

#include <constyaml/yaml.h>

using namespace constyaml;
using namespace constyaml::literals;
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

constexpr bool parse_true_false(const std::string_view s) {
  if(s == "true") return true;
  if(s != "false") parser_error("expected true or false");
  return false;
}

template <dict dict>
struct config_impl {
  static constexpr auto require(auto key, auto parse) { return parse(get<dict>(key)); }
  static constexpr auto require(auto key) { return get<dict>(key); }

  static constexpr int apple = require("apple"_k, parse_unsigned);
  // static constexpr auto apple = require("apple"_k, parse_true_false); // error!
  static constexpr bool banana = require("banana"_k, parse_true_false);
  static constexpr string_literal orange = require("orange"_k);
};

using config = config_impl<"apple: 714\nbanana: true\norange: 2\n"_yaml>;

int main() {
  std::cout << config::apple << std::endl;
  std::cout << config::banana << std::endl;
  std::cout << config::orange << std::endl;

  return 0;
}
