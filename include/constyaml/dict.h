#pragma once

#include "list.h"

namespace constyaml {
  namespace detail {
    template <std::size_t I, typename K, typename V>
    tag<K> dict_key_tag(const list_leaf<I, std::pair<K, V>>);

    template <std::size_t I, typename K, typename V>
    tag<V> dict_value_tag(const list_leaf<I, std::pair<K, V>>);
  }

  template <typename K, typename V>
  struct dict;

  template <typename... K, typename... V>
  struct dict<pack<K...>, pack<V...>>: list<std::pair<K, V>...> {
    constexpr dict(auto... args): list<std::pair<K,V>...>(args...) {}
  };

  template <typename... K, typename... V>
  dict(std::pair<K, V>...) -> dict<pack<K...>, pack<V...>>;

  template <std::size_t I, typename ConstDict>
  using dict_key = decltype(detail::dict_key_tag<I>(std::declval<ConstDict>()));

  template <std::size_t I, typename ConstDict>
  using dict_key_t = typename dict_key<I, ConstDict>::type;

  template <std::size_t I, typename ConstDict>
  using dict_value = decltype(detail::dict_value_tag<I>(std::declval<ConstDict>()));

  template <std::size_t I, typename ConstDict>
  using dict_value_t = typename dict_value<I, ConstDict>::type;

  template <std::size_t I, typename K, typename V>
  constexpr auto get_key(const detail::list_leaf<I, std::pair<K, V>>& dict) {
    return dict.value.first;
  }

  template <std::size_t I, typename K, typename V>
  constexpr auto get_value(const detail::list_leaf<I, std::pair<K, V>>& dict) {
    return dict.value.second;
  }

  template <dict dict>
  constexpr auto get(auto callable) {
    constexpr int idx = [&]<std::size_t... I>(std::index_sequence<I...>) {
      constexpr auto key = callable();
      int ret;
      ((get_key<I>(dict) == key && (ret = I)) || ... );
      return ret;
    }(std::make_index_sequence<dict.N>{});

    return get_value<idx>(dict);
  }
}
