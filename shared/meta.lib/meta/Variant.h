#pragma once
#include "Overloaded.h"
#include "TypeList.h"
#include "TypeTraits.h"
#include "ValueList.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace meta {

template<class... T>
struct VariantIndex {
private:
    using This = VariantIndex;
    size_t m{sizeof...(T)};

public:
    constexpr VariantIndex() = default;

    constexpr explicit VariantIndex(size_t v)
        : m(v) {}

    constexpr explicit operator bool() const { return m < sizeof...(T); }
    constexpr size_t value() const { return m; }

    constexpr bool operator==(const This& o) const = default;

    template<class... C>
    constexpr bool holds() const {
        using TypeList = TypeList<T...>;
        using Indices = IndexList<TypeList::indexOf(Type<C>{})...>;
        return Indices::contains(m);
    }
};

// workaround for:
// https://developercommunity.visualstudio.com/content/problem/1067936/-ambiguous-base-class-with-stdclatest.html
struct VisitFallback {
    template<class... Ts>
    constexpr auto operator()(const Ts&...) const {}
};
inline constexpr auto fallback_lambda = VisitFallback{};

template<class... T>
struct Variant {
private:
    using This = Variant;

public:
    using Data = std::variant<T...>;

private:
    Data m;

public:
    Variant() = default;

    template<class... A>
    requires(sizeof...(A) != 1 || !meta::same_remove_const_ref_head_type<Variant, A...>) &&
        std::is_constructible_v<Data, A...> //
        Variant(A&&... a)
        : m(std::forward<A>(a)...) {}

    // note: templated constructors are not forwarded with using
#define META_VARIANT_CONSTRUCT(Derived, Variant)                                                                       \
    template<class... A>                                                                                               \
    requires(sizeof...(A) != 1 || !meta::same_remove_const_ref_head_type<Derived, A...>) &&                            \
        std::is_constructible_v<Variant, A...> Derived(A&&... a)                                                       \
        : Variant(std::forward<A>(a)...) {}

    bool operator==(const This& o) const = default;

    constexpr static auto optionCount() { return sizeof...(T); }

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
        return std::visit(Overloaded{std::forward<F>(f)..., fallback_lambda}, m);
    }

    template<class... F>
    auto visitSome(F&&... f) & -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)..., fallback_lambda}, m);
    }

    template<class... F>
    auto visitSome(F&&... f) && -> decltype(auto) {
        return std::visit(Overloaded{std::forward<F>(f)..., fallback_lambda}, std::move(m));
    }

    template<class R>
    auto get(Type<R> = {}) const& -> decltype(auto) {
        return std::get<R>(m);
    }
    template<class R>
    auto get(Type<R> = {}) & -> decltype(auto) {
        return std::get<R>(m);
    }
    template<class R>
    auto get(Type<R> = {}) && -> decltype(auto) {
        return std::get<R>(std::move(m));
    }

    // allows to check for multiple types
    template<class... C>
    bool holds() const {
        return (std::holds_alternative<C>(m) || ...); // C++17
    }

    using Index = VariantIndex<T...>;

    auto index() const -> Index { return Index(m.index()); }

    template<class C>
    constexpr static auto indexOf() -> decltype(auto) {
        return Index(TypeList<T...>::indexOf(Type<C>{}));
    }
};

} // namespace meta
