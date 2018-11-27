#pragma once
#include "Variant.h"

#include <array>
#include <ostream>
#include <type_traits>

namespace meta {

template<class T, class... V>
constexpr bool is_all_same(Type<T> = {}, TypeList<V...> = {}) {
    return (std::is_same_v<T, V> && ...);
}

namespace details {

template<class... T>
constexpr auto isAllNamed() -> decltype(is_all_same<const char*, decltype(nameOf(Type<T>{}))...>()) {
    return is_all_same<const char*, decltype(nameOf(Type<T>{}))...>();
}

template<class Out, class Index, class Enable = void>
struct PrintTypeNames {
    auto operator()(Out& out, Index) -> Out& { return out; }
};

template<class Out, class... T>
struct PrintTypeNames<Out, VariantIndex<T...>, std::enable_if_t<isAllNamed<T...>()>> {

    auto operator()(Out& out, VariantIndex<T...> idx) -> Out& {
        constexpr std::array<const char*, sizeof...(T)> names = {nameOf(Type<T>{})...};
        return out << '[' << names[idx.value()] << ']';
    }
};

} // namespace details

template<typename Char, typename CharTraits, class... T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, meta::VariantIndex<T...> idx) //
    -> std::enable_if_t<meta::details::isAllNamed<T...>(), ::std::basic_ostream<Char, CharTraits>&> {

    constexpr std::array<const char*, sizeof...(T)> names = {nameOf(meta::Type<T>{})...};
    return out << names[idx.value()];
}

template<typename Char, typename CharTraits, class... T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const meta::Variant<T...>& var) //
    -> std::enable_if_t<
        meta::is_all_same<decltype(out), decltype(out << std::declval<T>())...>(),
        ::std::basic_ostream<Char, CharTraits>&> {

    meta::details::PrintTypeNames<decltype(out), meta::VariantIndex<T...>>{}(out, var.index());
    return var.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

} // namespace meta
