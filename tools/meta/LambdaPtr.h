#pragma once

namespace meta {

template<class F>
struct LambdaPtr;

template<class R, class T, class... Args>
struct LambdaPtr<R (T::*)(Args...) const> {
    using L = R(Args...);
    using P = L*;

    constexpr auto operator()(L l) { return static_cast<P>(l); }
};

// use this to cast a lambda to a function pointer
template<class F>
constexpr auto lambdaPtr(F f) {
    return LambdaPtr<decltype(&F::operator())>{}(f);
}

} // namespace meta
