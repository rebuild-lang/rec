#pragma once
#include "Overloaded.h"
#include "TypeList.h"
#include "ValueList.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace meta {

template<class... T>
class Variant {
    using This = Variant;
    using Data = std::variant<T...>;
    Data m{};

public:
    Variant() = default;

    template<class... A, typename = std::enable_if_t<std::is_constructible_v<Data, A...>>>
    Variant(A&&... a)
        : m(std::forward<A>(a)...) {}

    // note: templated constructors are not forwarded with using
#define META_VARIANT_CONSTRUCT(Derived, Variant)                                                                       \
    using Variant::Variant;                                                                                            \
    template<class... A, typename = std::enable_if_t<std::is_constructible_v<Variant, A...>>>                          \
    Derived(A&&... a)                                                                                                  \
        : Variant(std::forward<A>(a)...) {}

    bool operator==(const This& o) const { return m == o.m; }
    bool operator!=(const This& o) const { return m != o.m; }

    template<class... F>
    auto visit(F&&... f) const& -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)...}, m);
    }

    template<class... F>
    auto visit(F&&... f) & -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)...}, m);
    }

    template<class... F>
    auto visit(F&&... f) && -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)...}, std::move(m));
    }

    template<class... F>
    auto visitSome(F&&... f) const& -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)..., [](const auto&) {}}, m);
    }

    template<class... F>
    auto visitSome(F&&... f) & -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)..., [](const auto&) {}}, m);
    }

    template<class... F>
    auto visitSome(F&&... f) && -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)..., [](const auto&) {}}, std::move(m));
    }

    template<class R>
    auto get(Type<R> = {}) const& -> decltype(auto) {
        return std::get<R>(m);
    }
    template<class R>
    auto get(Type<R> = {}) & -> decltype(auto) {
        return std::get<R>(m);
    }

    // allows to check for multiple types
    template<class... C>
    bool holds() const {
        return (std::holds_alternative<C>(m) || ...); // C++17
    }

    class Index {
        using This = Index;
        size_t m{sizeof...(T)};

    public:
        constexpr Index() = default;

        constexpr explicit Index(size_t v)
            : m(v) {}

        constexpr operator bool() const { return m < sizeof...(T); }

        constexpr bool operator==(const This& o) { return m == o.m; }
        constexpr bool operator!=(const This& o) { return m != o.m; }

        template<class... C>
        constexpr bool holds() const {
            using TypeList = TypeList<T...>;
            using Indices = IndexList<TypeList::indexOf(Type<C>{})...>;
            return Indices::contains(m);
        }
    };

    auto index() const -> decltype(auto) { return Index(m.index()); }

    template<class C>
    constexpr static auto indexOf() -> decltype(auto) {
        return Index(TypeList<T...>::indexOf(Type<C>{}));
    }
};

} // namespace meta
