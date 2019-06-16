#pragma once
#include "Type.h"

#include "meta/Flags.h"

#include <cinttypes>

namespace intrinsic {

enum class ParameterFlag : uint64_t {
    Assignable = 1 << 0,
    Unrolled = 1 << 1,
    Reference = 1 << 2,
    Optional = 1 << 3,
};
using ParameterFlags = meta::Flags<ParameterFlag>;
META_FLAGS_OP(ParameterFlags)

enum class ParameterSide { Left, Right, Result, Implicit };

struct ParameterInfo {
    Name name{};
    ParameterSide side{};
    ParameterFlags flags{};
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

// used to extract all types of parameters
template<class T>
struct Parameter {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;
    static constexpr auto info() -> ParameterInfo { return T::info(); }
    static constexpr auto typeInfo() -> TypeInfo { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.none(ParameterFlag::Assignable), "value parameter is not assignable");
    static_assert(info().flags.none(ParameterFlag::Reference), "value parameter is not a reference");
};

template<class T>
struct Parameter<const T&> {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;

    static constexpr auto info() -> ParameterInfo { return T::info(); }
    static constexpr auto typeInfo() { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.none(ParameterFlag::Assignable), "const-reference parameter is not assignable");
    static_assert(info().flags.any(ParameterFlag::Reference), "const-reference uses reference");
};

template<class T>
struct Parameter<T&> {
    using type = details::NestedType<T>;
    static constexpr bool is_pointer = std::is_pointer_v<type>;

    static constexpr auto info() -> ParameterInfo { return T::info(); }
    static constexpr auto typeInfo() { return details::ExistingTypeOf<type>::info(); }

    static_assert(info().flags.any(ParameterFlag::Assignable), "reference parameter has to be assignable");
    static_assert(info().flags.none(ParameterFlag::Reference), "reference uses reference");
};

} // namespace intrinsic
