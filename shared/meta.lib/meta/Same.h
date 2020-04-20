#pragma once

namespace meta {

template <class A, class B>
constexpr auto same = false;

template<class A>
constexpr auto same<A, A> = true;

}
