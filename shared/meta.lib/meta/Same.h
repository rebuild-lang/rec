#pragma once

namespace meta {

template<class, class>
constexpr auto same = false;

template<class A>
constexpr auto same<A, A> = true;

} // namespace meta
