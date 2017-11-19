#pragma once
#include "overloaded.h"
#include "type_list.h"
#include "value_list.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace meta {

template<class... T>
class variant {
    using this_t = variant;
    std::variant<T...> m;

public:
    variant() = default;
    variant(const this_t &) = default;
    this_t &operator=(const this_t &) = default;
    variant(this_t &&) = default;
    this_t &operator=(this_t &&) = default;

    template<
        class S,
        class... A, // 1+ arguments
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, this_t>> // ensure this does not capture a copy
        >
    variant(S &&s, A &&... a)
        : m(std::forward<S>(s), std::forward<A>(a)...) {}

    bool operator==(const this_t &o) const { return m == o.m; }
    bool operator!=(const this_t &o) const { return m != o.m; }

    template<class... F>
    auto visit(F &&... f) const & -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)...), m);
    }

    template<class... F>
    auto visit(F &&... f) & -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)...), m);
    }

    template<class... F>
    auto visit(F &&... f) && -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)...), std::move(m));
    }

    template<class... F>
    auto visit_some(F &&... f) const & -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)..., [](const auto &) {}), m);
    }

    template<class... F>
    auto visit_some(F &&... f) & -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)..., [](auto &) {}), m);
    }

    template<class... F>
    auto visit_some(F &&... f) && -> decltype(auto) {
        return std::visit(make_overloaded(std::forward<F>(f)..., [](auto &&) {}), std::move(m));
    }

    template<class R>
    auto get(type_t<R> = {}) const & -> decltype(auto) {
        return std::get<R>(m);
    }
    template<class R>
    auto get(type_t<R> = {}) & -> decltype(auto) {
        return std::get<R>(m);
    }

    // allows to check for multiple types
    template<class... C>
    bool holds() const {
        // return std::holds_alternative<C>(m) || ...; // C++17
        bool sum = false;
        auto x = {(sum = sum || std::holds_alternative<C>(m), 0)...};
        (void)x;
        return sum;
    }

    class index_t {
        using this_t = index_t;
        size_t m;

    public:
        constexpr index_t()
            : m(sizeof...(T)) {}
        constexpr index_t(const this_t &) = default;
        constexpr this_t &operator=(const this_t &) = default;
        constexpr index_t(this_t &&) = default;
        constexpr this_t &operator=(this_t &&) = default;

        constexpr explicit index_t(size_t v)
            : m(v) {}

        constexpr operator bool() const { return m < sizeof...(T); }

        constexpr bool operator==(const this_t &o) { return m == o.m; }
        constexpr bool operator!=(const this_t &o) { return m != o.m; }

        template<class... C>
        constexpr bool holds() const {
            using type_list = type_list_t<T...>;
            using indices = index_list<type_list::index_of(type_t<C>{})...>;
            return indices::contains(m);
        }
    };

    auto index() const -> decltype(auto) { return index_t(m.index()); }

    template<class C>
    constexpr static auto index_of() -> decltype(auto) {
        return index_t(type_list_t<T...>::index_of(type_t<C>{}));
    }
};

} // namespace meta
