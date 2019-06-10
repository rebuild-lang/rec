#pragma once
#include "Type.h"

#include <type_traits>

namespace meta {

template<class...>
struct TypePack {};

/// Equality
template<class... As, class... Bs>
constexpr bool operator==(TypePack<As...>, TypePack<Bs...>) {
    if constexpr (sizeof...(As) != sizeof...(Bs))
        return false;
    else
        return ((Type<As>{} == Type<Bs>{}) && ...);
}
template<class... As, class... Bs>
constexpr bool operator!=(TypePack<As...> a, TypePack<Bs...> b) {
    return !(a == b);
}

/// Wrap meta function
template<class... Ts>
struct GetTypePack {
    using Return = TypePack<ToBareType<Ts>...>;
};
template<template<class...> class Template, class... Ts>
struct GetTypePack<Template<Ts...>> {
    using Return = TypePack<Ts...>;
};
template<template<class...> class Template, class... Ts>
struct GetTypePack<Type<Template<Ts...>>> {
    using Return = TypePack<Ts...>;
};
template<class... Ts>
using ToTypePack = typename GetTypePack<Ts...>::Return;

/// Allow unwrap single element
template<class T>
struct GetBareType<TypePack<T>> {
    using Return = T;
};

/// Extraction helpers
template<class... Ts>
constexpr auto count(TypePack<Ts...> = {}) -> size_t {
    return sizeof...(Ts);
}
template<class H, class... T>
constexpr auto head(TypePack<H, T...> = {}) -> Type<H> {
    return {};
}
template<class H, class... T>
constexpr auto tail(TypePack<H, T...> = {}) -> TypePack<T...> {
    return {};
}

/// Concatenations
template<class... As, class... Bs>
constexpr auto operator+(TypePack<As...>, TypePack<Bs...>) -> TypePack<As..., Bs...> {
    return {};
}
template<class... As, class B>
constexpr auto operator+(TypePack<As...>, Type<B>) -> TypePack<As..., B> {
    return {};
}
template<class A, class... Bs>
constexpr auto operator+(Type<A>, TypePack<Bs...>) -> TypePack<A, Bs...> {
    return {};
}

/// return true if candidate type is part of the pack
template<class Candidate, class... Ts>
constexpr auto contains(TypePack<Ts...>, Type<Candidate> = {}) {
    return ((Type<Ts>{} == Type<Candidate>{}) || ...);
}

/// return number of occurences of candidate type in the TypePack
template<class Candidate, class... Ts>
constexpr auto countOf(TypePack<Ts...>, Type<Candidate> = {}) -> size_t {
    return ((Type<Ts>{} == Type<Candidate>{} ? 1 : 0) + ... + 0);
}

/// Iteration
template<class Callable, class... Ts>
constexpr void ForEach(Callable&& callable, TypePack<Ts...>) {
    (..., callable(Type<Ts>{}));
}

/// Next to useless in MSVC, can't deal with lambda in decltype/constexpr
template<class Callable, class... Ts>
constexpr auto filter(Callable&& predicate, TypePack<Ts...>) {
    return (... + std::conditional_t<predicate(Type<Ts>{}), TypePack<Ts>, TypePack<>>{});
}

template<class N, class... Ts>
constexpr auto remove(TypePack<Ts...>, Type<N> = {}) {
    return (... + (std::conditional_t<std::is_same_v<Ts, N>, TypePack<>, TypePack<Ts>>{}));
}

template<class... Ts>
constexpr auto isUnique(TypePack<Ts...> = {}) {
    if constexpr (0 != count<Ts...>()) {
        constexpr auto h = head<Ts...>();
        constexpr auto t = tail<Ts...>();
        return !contains(t, h) && isUnique(t);
    }
    else
        return true;
}

template<class... Ts>
constexpr auto makeUnique(TypePack<Ts...> = {}) {
    if constexpr (0 != count<Ts...>()) {
        constexpr auto h = head<Ts...>();
        constexpr auto t = tail<Ts...>();
        if constexpr (contains(t, h))
            return makeUnique(t);
        else
            return h + makeUnique(t);
    }
    else
        return TypePack{};
}

} // namespace meta
