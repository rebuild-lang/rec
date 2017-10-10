#pragma once
#include <utility>

namespace meta {

//#if __cplusplus > 201700L

// template<class... Ts>
// struct overloaded : Ts... {
//    using Ts::operator()...;
//};

// // template deduction guide
// template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

//#else

/// C++14 implementation of overloaded lambda set
// required for easy usage of std::variant<>
template<class... Ts>
struct overloaded;

template<class T>
struct overloaded<T> : T {
    overloaded(T &&t)
        : T(std::forward<T>(t)) {}
    using T::operator();
};
template<class T, class... Ts>
struct overloaded<T, Ts...> : T, overloaded<Ts...> {
    overloaded(T &&t, Ts &&... ts)
        : T(std::forward<T>(t))
        , overloaded<Ts...>(std::forward<Ts>(ts)...) {}

    using T::operator();
    using overloaded<Ts...>::operator();
};

template<class... Ts>
auto make_overloaded(Ts &&... ts) {
    return overloaded<Ts...>(std::forward<Ts>(ts)...);
}

//#endif

} // namespace meta
