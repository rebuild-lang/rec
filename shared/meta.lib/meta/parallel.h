#pragma once
#include <tuple>
#include <utility>

namespace meta {

namespace detail {

template<class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
}

} // namespace detail

template<class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return detail::apply_impl(
        std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
}

template<class... C>
struct containers_view {
    using index_t = std::make_index_sequence<sizeof...(C)>;

    struct iterator {
        auto operator++() & -> iterator& {
            apply([](auto&... it) { return std::tuple{++it...}; }, cit);
            return *this;
        }

        auto operator*() const -> std::tuple<typename C::reference...> {
            return apply([](auto&... it) { return std::tuple{*it...}; }, cit);
        }

        std::tuple<typename C::iterator...> cit;
    };

    auto begin() -> iterator {
        return apply([](auto&... c) { return std::tuple{c.begin()...}; }, c);
    }

    auto end() -> iterator {
        return apply([](auto&... c) { return std::tuple{c.end()...}; }, c);
    }

    std::tuple<std::reference_wrapper<C>...> c;
};

template<class... C>
auto parallel(C&... c) -> containers_view<C...> {
    return {c...};
}

} // namespace meta
