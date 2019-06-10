#pragma once
#include "Type.h"

#include "meta/Flags.h"

#include <cinttypes>

namespace intrinsic {

enum class ArgumentFlag : uint64_t {
    Assignable = 1 << 0,
    Unrolled = 1 << 1,
    Reference = 1 << 2,
    Optional = 1 << 3,
};
using ArgumentFlags = meta::Flags<ArgumentFlag>;
META_FLAGS_OP(ArgumentFlags)

enum class ArgumentSide { Left, Right, Result, Implicit };

struct ArgumentInfo {
    Name name{};
    ArgumentSide side{};
    ArgumentFlags flags{};
};

namespace details {

template<class T, class = void>
struct NestedTypeImpl {
    using type = T;
};

template<class T>
struct NestedTypeImpl<T, std::enable_if_t<sizeof(T) == sizeof(T::v)>> {
    using type = decltype(std::declval<T>().v);
};

template<class T>
using NestedType = typename NestedTypeImpl<T>::type;

template<class T, class = void>
struct ExistingTypeOfImpl {
    using type = TypeOf<T>;
};

template<class T>
struct ExistingTypeOfImpl<T*, std::enable_if_t<&TypeOf<T>::info>> {
    using type = TypeOf<T>;
};

template<class T>
using ExistingTypeOf = typename ExistingTypeOfImpl<T>::type;

} // namespace details

// used to extract all types of arguments
template<class T>
struct Argument {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;
    static constexpr auto info() -> ArgumentInfo { return T::info(); }
    static constexpr auto typeInfo() -> TypeInfo { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.none(ArgumentFlag::Assignable), "value argument is not assignable");
    static_assert(info().flags.none(ArgumentFlag::Reference), "value argument is not a reference");
};

template<class T>
struct Argument<const T&> {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;

    static constexpr auto info() -> ArgumentInfo { return T::info(); }
    static constexpr auto typeInfo() { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.none(ArgumentFlag::Assignable), "const-reference argument is not assignable");
    static_assert(info().flags.any(ArgumentFlag::Reference), "const-reference uses reference");
};

template<class T>
struct Argument<T&> {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;

    static constexpr auto info() -> ArgumentInfo { return T::info(); }
    static constexpr auto typeInfo() { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.any(ArgumentFlag::Assignable), "reference argument has to be assignable");
    static_assert(info().flags.none(ArgumentFlag::Reference), "reference uses reference");
};

} // namespace intrinsic
