#pragma once

namespace strings {

template<class Container, class Delim>
struct JoinEach {
    const Container& container;
    const Delim& delim;
};

template<class Out, class Container, class Delim>
inline auto operator<<(Out& out, JoinEach<Container, Delim> j) -> Out& {
    auto i = 0u;
    auto l = j.container.size();
    for (const auto& e : j.container) {
        out << e;
        if (++i != l) out << j.delim;
    }
    return out;
}

template<class Container, class Delim>
auto joinEach(const Container& container, const Delim& delim) -> JoinEach<Container, Delim> {
    return {container, delim};
}

template<class Container, class Delim, class F>
struct JoinEachApply {
    const Container& container;
    const Delim& delim;
    const F& f;
};

template<class Out, class Container, class Delim, class F>
inline auto operator<<(Out& out, JoinEachApply<Container, Delim, F> j) -> Out& {
    auto i = 0u;
    auto l = j.container.size();
    for (const auto& e : j.container) {
        j.f(e);
        if (++i != l) out << j.delim;
    }
    return out;
}

template<class Container, class Delim, class F>
auto joinEachApply(const Container& container, const Delim& delim, const F& f) -> JoinEachApply<Container, Delim, F> {
    return {container, delim, f};
}

} // namespace strings
