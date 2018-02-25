#pragma once

namespace meta {

//#if __cplusplus > 201700L

// template<class... Ts>
// struct Overloaded : Ts... {
//    using Ts::operator()...;
//};

// // template deduction guide
// template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

//#else
#include <utility>

/// C++14 implementation of overloaded lambda set
// required for easy usage of std::variant<>
template<class... Ts>
struct Overloaded;

template<class T>
struct Overloaded<T> : T {
    Overloaded(T &&t)
        : T(std::forward<T>(t)) {}
    using T::operator();
};
template<class T, class... Ts>
struct Overloaded<T, Ts...> : T, Overloaded<Ts...> {
    Overloaded(T &&t, Ts &&... ts)
        : T(std::forward<T>(t))
        , Overloaded<Ts...>(std::forward<Ts>(ts)...) {}

    using T::operator();
    using Overloaded<Ts...>::operator();
};

template<class... Ts>
auto makeOverloaded(Ts &&... ts) {
    return Overloaded<Ts...>(std::forward<Ts>(ts)...);
}

//#endif

} // namespace meta
