#pragma once

#include "const_list.h"

namespace constyaml {
  namespace detail {
    template <std::size_t I, typename K, typename V>
    tag<K> const_dict_key_tag(const const_list_leaf<I, std::pair<K, V>>);

    template <std::size_t I, typename K, typename V>
    tag<V> const_dict_value_tag(const const_list_leaf<I, std::pair<K, V>>);
  }

  template <typename K, typename V>
  struct const_dict;

  template <typename... K, typename... V>
  struct const_dict<pack<K...>, pack<V...>>: const_list<std::pair<K, V>...> {
    constexpr const_dict(auto... args): const_list<std::pair<K,V>...>(args...) {}
  };

  template <typename... K, typename... V>
  const_dict(std::pair<K, V>...) -> const_dict<pack<K...>, pack<V...>>;

  template <std::size_t I, typename ConstDict>
  using const_dict_key = decltype(detail::const_dict_key_tag<I>(std::declval<ConstDict>()));

  template <std::size_t I, typename ConstDict>
  using const_dict_key_t = typename const_dict_key<I, ConstDict>::type;

  template <std::size_t I, typename ConstDict>
  using const_dict_value = decltype(detail::const_dict_value_tag<I>(std::declval<ConstDict>()));

  template <std::size_t I, typename ConstDict>
  using const_dict_value_t = typename const_dict_value<I, ConstDict>::type;

  template <std::size_t I, typename K, typename V>
  constexpr auto get_key(const detail::const_list_leaf<I, std::pair<K, V>>& dict) {
    return dict.value.first;
  }

  template <std::size_t I, typename K, typename V>
  constexpr auto get_value(const detail::const_list_leaf<I, std::pair<K, V>>& dict) {
    return dict.value.second;
  }

  template <const_dict dict>
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
