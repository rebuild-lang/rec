#pragma once
#include "TypePack.h"
#include "TypeTraits.h"
#include "Variant.h"

#include <array>
#include <ostream>
#include <type_traits>

namespace meta {

namespace details {

template<class P, class V = void>
constexpr bool all_named = false;

template<class... T>
constexpr bool all_named<TypePack<T...>, std::enable_if_t<all_same_type<const char*, decltype(nameOf(type<T>))...>>> =
    true;

template<class Out, class Index, class Enable = void>
struct PrintTypeNames {
    auto operator()(Out& out, Index) -> Out& { return out; }
};

template<class Out, class... T>
struct PrintTypeNames<Out, VariantIndex<T...>, std::enable_if_t<all_named<TypePack<T...>>>> {

    auto operator()(Out& out, VariantIndex<T...> idx) -> Out& {
        constexpr std::array<const char*, sizeof...(T)> names = {nameOf(type<T>)...};
        return out << '[' << names[idx.value()] << ']';
    }
};

} // namespace details

template<typename Char, typename CharTraits, class... T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, meta::VariantIndex<T...> idx) //
    -> std::enable_if_t<meta::details::all_named<TypePack<T...>>, ::std::basic_ostream<Char, CharTraits>&> {

    constexpr std::array<const char*, sizeof...(T)> names = {nameOf(meta::type<T>)...};
    return out << names[idx.value()];
}

template<typename Char, typename CharTraits, class... T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const meta::Variant<T...>& var) //
    -> std::enable_if_t<
        all_same_type<decltype(out), decltype(out << std::declval<T>())...>,
        ::std::basic_ostream<Char, CharTraits>&> {

    meta::details::PrintTypeNames<decltype(out), meta::VariantIndex<T...>>{}(out, var.index());
    return var.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

} // namespace meta
