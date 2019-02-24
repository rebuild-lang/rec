#pragma once
#include <cstddef>

namespace meta {

template<class T>
struct Type {};

/// Equality
template<class A, class B>
constexpr bool operator==(Type<A>, Type<B>) {
    return false;
}
template<class A>
constexpr bool operator==(Type<A>, Type<A>) {
    return true;
}
template<class A, class B>
constexpr bool operator!=(Type<A> a, Type<B> b) noexcept {
    return !(a == b);
}

/// Unwrap meta function
template<class T>
struct GetBareType {
    using Return = T;
};
template<class T>
struct GetBareType<const T> {
    using Return = typename GetBareType<T>::Return;
};
template<class T>
struct GetBareType<Type<T>> {
    using Return = T;
};
template<class T>
using ToBareType = typename GetBareType<T>::Return;

/// Wrap meta function
template<class T>
using ToType = Type<ToBareType<T>>;

/// Type Usage
template<class A>
constexpr auto sizeOf(Type<A> = {}) noexcept -> size_t {
    return sizeof(A);
}

} // namespace meta
