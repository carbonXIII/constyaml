#pragma once

#include <utility>
#include <variant>

#include "types.h"

// tuple is not structural (yet)
// so we need to define our own tuple type
// https://stackoverflow.com/a/69194245/3865952

namespace constyaml {
  namespace detail {
    template <std::size_t I, typename T>
    struct const_list_leaf {
      T value;
    };

    template <typename Seq, typename T>
    struct const_list_impl;

    template <std::size_t... I, typename... T>
    struct const_list_impl<std::index_sequence<I...>, pack<T...>>: const_list_leaf<I,T>... {
      static constexpr int N = sizeof...(T);
      constexpr const_list_impl(auto... args): const_list_leaf<I,T>{args}... {}
    };

    template <typename... T>
    const_list_impl(T...) -> const_list_impl<std::make_index_sequence<sizeof...(T)>, pack<T...>>;

    template <std::size_t I, typename T>
    tag<T> const_list_element_tag(const const_list_leaf<I, T>&);
  }

  template <std::size_t I, typename T>
  constexpr const T& get(const detail::const_list_leaf<I, T>& v) {
    return v.value;
  }

  template <std::size_t I, typename ConstList>
  using const_list_element = decltype(detail::const_list_element_tag<I>(std::declval<ConstList>()));

  template <std::size_t I, typename ConstList>
  using const_list_element_t = typename const_list_element<I, ConstList>::type;

  template <typename... T>
  using const_list = detail::const_list_impl<std::make_index_sequence<sizeof...(T)>, pack<T...>>;
}
