module;

#include <utility>
#include <variant>
#include "types.h"

export module constyaml.list;

// tuple is not structural (yet)
// so we need to define our own tuple type
// https://stackoverflow.com/a/69194245/3865952

namespace constyaml::detail {
  export template <std::size_t I, typename T>
  struct list_leaf {
    T value;
  };

  template <typename Seq, typename T>
  struct list_impl;

  template <std::size_t... I, typename... T>
  struct list_impl<std::index_sequence<I...>, pack<T...>>: list_leaf<I,T>... {
    static constexpr int N = sizeof...(T);
    constexpr list_impl(auto... args): list_leaf<I,T>{args}... {}
  };

  template <typename... T>
  list_impl(T...) -> list_impl<std::make_index_sequence<sizeof...(T)>, pack<T...>>;

  template <std::size_t I, typename T>
  tag<T> list_element_tag(const list_leaf<I, T>&);
}

export namespace constyaml {
  template <std::size_t I, typename T>
  constexpr const T& get(const detail::list_leaf<I, T>& v) {
    return v.value;
  }

  template <std::size_t I, typename ConstList>
  using list_element = decltype(detail::list_element_tag<I>(std::declval<ConstList>()));

  template <std::size_t I, typename ConstList>
  using list_element_t = typename list_element<I, ConstList>::type;

  template <typename... T>
  using list = detail::list_impl<std::make_index_sequence<sizeof...(T)>, pack<T...>>;
}
