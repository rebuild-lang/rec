#pragma once
#include "Type.h"

#include "tools/meta/Flags.h"

#include <cinttypes>

namespace intrinsic {

enum class ArgumentFlag : uint64_t {
    Assignable = 1 << 0,
    Unrolled = 1 << 1,
};
using ArgumentFlags = meta::Flags<ArgumentFlag>;
META_FLAGS_OP(ArgumentFlags)

enum class ArgumentSide { Left, Right, Result };

struct ArgumentInfo {
    const char* name{};
    ArgumentSide side{};
    ArgumentFlags flags{};
};

namespace details {

template<class T, class = void>
struct NestedTypeImpl {
    using type = T;
};

template<class T>
struct NestedTypeImpl<T, std::enable_if_t<sizeof(T) == sizeof(std::declval<T>().v)>> {
    using type = decltype(std::declval<T>().v);
};

template<class T>
using NestedType = typename NestedTypeImpl<T>::type;

} // namespace details

// used to extract all types of arguments
template<class T>
struct Argument {
    using type = details::NestedType<T>;
    static constexpr auto info() { return T::info(); }
    static constexpr auto typeInfo() { return TypeOf<type>::info(); }
    static_assert(!info().flags.any(ArgumentFlag::Assignable), "non-reference argument is not assignable");
};

template<class T>
struct Argument<const T&> {
    using type = details::NestedType<T>;
    static constexpr auto info() { return T::info(); }
    static constexpr auto typeInfo() { return TypeOf<type>::info(); }
    static_assert(!info().flags.any(ArgumentFlag::Assignable), "const-reference argument is not assignable");
};
template<class T>
struct Argument<T&> : T {
    using type = details::NestedType<T>;
    static constexpr auto info() { return T::info(); }
    static constexpr auto typeInfo() { return TypeOf<type>::info(); }
    static_assert(info().flags.any(ArgumentFlag::Assignable), "reference argument has to be assignable");
};

} // namespace intrinsic
