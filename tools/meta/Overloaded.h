#pragma once

namespace meta {

template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

// template deduction guide
template<class... Ts>
Overloaded(Ts...)->Overloaded<Ts...>;

} // namespace meta
