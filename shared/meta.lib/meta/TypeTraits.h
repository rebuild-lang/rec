#pragma once
#include "Type.h"
#include "TypePack.h"

namespace meta {

// simpler than std::integral_constant<bool, X>
template<bool V>
struct Bool {
    constexpr operator bool() const { return V; }
};

using True = Bool<true>;
using False = Bool<false>;

// faster than std::is_same_v
template<class, class>
constexpr bool same_type = false;
template<class A>
constexpr bool same_type<A, A> = true;

// true if all types equal the first type
template<class A, class... Bs>
constexpr bool all_same_type = (same_type<A, Bs> && ... && true);

// true if first type of a pack equals
template<class, class, class...>
constexpr bool same_head_type = false;
template<class A, class... Bs>
constexpr bool same_head_type<A, A, Bs...> = true;

// true only if T is const
template<class T>
constexpr bool is_const = false;
template<class T>
constexpr bool is_const<const T> = true;

static_assert(is_const<int const>);
static_assert(!is_const<int>);

// true only if T is reference
template<class T>
constexpr bool is_reference = false;
template<class T>
constexpr bool is_reference<T&> = true;

static_assert(!is_reference<int>);
static_assert(is_reference<int&>);

// true only if T is pointer
template<class T>
constexpr bool is_pointer = false;
template<class T>
constexpr bool is_pointer<T*> = true;

static_assert(!is_pointer<int>);
static_assert(is_pointer<int*>);

// simple std::declval without special rvalue support
template<class V>
auto declVal() -> V;

// faster than std::is_constructible_v (but with TypePack for args)
template<class T, class Args, class E = T>
constexpr bool is_constructible = false;

template<class T, class... Args>
constexpr bool is_constructible<T, TypePack<Args...>, decltype(T{declVal<Args>()...})> = true;

// new
template<class T, class Args, class E = void>
constexpr bool is_implicit_constructible = false;

template<class T, class... Args>
constexpr bool is_implicit_constructible<T, TypePack<Args...>, decltype(declVal<void (*)(T)>()({declVal<Args>()...}))> =
    true;

// note: we cannot check if move constructor is present, but we can check the move assignment operator
// is move assignable
template<class T, class E = T&>
constexpr bool has_move_assignment = false;

template<class T>
constexpr bool has_move_assignment<
    T, //
    decltype((declVal<T>().*(static_cast<T& (T::*)(T&&)>(&T::operator=)))(declVal<T>()))> = true;

struct MoveOnly {
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&) = default;
};
constexpr auto foo = static_cast<MoveOnly& (MoveOnly::*)(MoveOnly&&)>(&MoveOnly::operator=);
using Foo = decltype((declVal<MoveOnly>().*foo)(declVal<MoveOnly>()));

struct CopyOnly {
    CopyOnly& operator=(const CopyOnly&) = default;
    CopyOnly& operator=(CopyOnly&&) = delete;
};
static_assert(has_move_assignment<MoveOnly>);
static_assert(!has_move_assignment<CopyOnly>);
static_assert(has_move_assignment<TypePack<int>>);

// - crashes GCC11
// template<class T>
// auto removeConstRef(Type<T>) -> T;
// template<class T>
// auto removeConstRef(Type<T const>) -> T;
// template<class T>
// auto removeConstRef(Type<T&>) -> T;
// template<class T>
// auto removeConstRef(Type<T const&>) -> T;
// template<class T>
// auto removeConstRef(Type<T&&>) -> T;
// template<class T>
// auto removeConstRef(Type<T const&&>) -> T;
// template<class T>
// using RemoveConstRef = decltype(removeConstRef(type<T>));

template<class T>
using RemoveConstRef = std::remove_const_t<std::remove_reference_t<T>>;

static_assert(same_type<int, RemoveConstRef<int>>);
static_assert(same_type<int, RemoveConstRef<int const>>);
static_assert(same_type<int, RemoveConstRef<int&>>);
static_assert(same_type<int, RemoveConstRef<int const&>>);
static_assert(same_type<int, RemoveConstRef<int&&>>);
static_assert(same_type<int, RemoveConstRef<int const&&>>);
//
template<class, class...>
constexpr bool same_remove_const_ref_head_type = false;
template<class A, class B, class... Bs>
constexpr bool same_remove_const_ref_head_type<A, B, Bs...> = same_type<A, RemoveConstRef<B>>;

static_assert(same_remove_const_ref_head_type<int, int const&, bool>);
static_assert(!same_remove_const_ref_head_type<TypePack<int>, int&&, bool>);

} // namespace meta
