#pragma once
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace meta {

/// extended optional based on std::optional
// simple encapsulation to add map functionality
// not all features are delegated!
template<class T>
class Optional {
    using This = Optional;
    std::optional<T> m{};

public:
    constexpr Optional() = default;
    constexpr Optional(const T& t)
        : m{t} {}
    constexpr Optional(T&& t)
        : m(std::move(t)) {}

    constexpr explicit operator bool() const { return m.has_value(); }
    constexpr auto value() const& -> decltype(auto) { return m.value(); }
    constexpr auto value() & -> decltype(auto) { return m.value(); }
    constexpr auto value() && -> decltype(auto) { return std::move(m).value(); }

    /// encapsulate the condition
    template<class F>
    constexpr auto map(F&& f) const& -> decltype(f(value())) {
        if (m.has_value()) return f(value());
        if constexpr (!std::is_void_v<decltype(f(value()))>) return {};
    }
    template<class F>
    constexpr auto map(F&& f) && -> decltype(f(std::move(m).value())) {
        if (m.has_value()) return f(std::move(m).value());
        if constexpr (!std::is_void_v<decltype(f(std::move(m).value()))>) return {};
    }

    /// default values
    template<class D>
    constexpr auto orValue(D&& d) const& -> decltype(auto) {
        if (m.has_value()) return m.value();
        return std::forward<D>(d);
    }
    template<class D>
    constexpr auto orValue(D&& d) & -> decltype(auto) {
        if (m.has_value()) return m.value();
        return std::forward<D>(d);
    }
    template<class D>
    constexpr auto orValue(D&& d) && -> decltype(auto) {
        if (m.has_value()) return std::move(m).value();
        return std::forward<D>(d);
    }

    template<class F>
    constexpr auto operator&&(F&& f)
        -> std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::declval<T>())), bool>, bool> {
        if (*this) return f(value());
        return false;
    }

    constexpr bool operator==(const This& o) const { return m == o.m; }
    constexpr bool operator!=(const This& o) const { return m != o.m; }
};

template<class T>
class Optional<T&> : public Optional<std::reference_wrapper<T>> {
    using This = Optional;
    using Base = Optional<std::reference_wrapper<T>>;

public:
    constexpr Optional() = default;
    constexpr Optional(const T& t)
        : Base{std::ref(t)} {}
    constexpr Optional(const std::reference_wrapper<T>& t)
        : Base{t} {}
};

/// tag type to trigger a value packed optional implementation
template<class InvalidFunc>
struct Packed;

/// very simple value packed optional specialization
// keep the API in sync with above optional
template<class InvalidFunc>
class Optional<Packed<InvalidFunc>> {
    using This = Optional;
    using Data = decltype(std::declval<InvalidFunc>()());
    Data data{InvalidFunc{}()};

public:
    constexpr Optional() = default;
    constexpr Optional(const Data& data)
        : data(data) {}

    constexpr explicit operator bool() const { return !(data == InvalidFunc{}()); }
    constexpr auto value() const& -> decltype(auto) { return data; }
    constexpr auto value() && -> decltype(auto) { return std::move(data); }
    constexpr auto value() & -> decltype(auto) { return data; }

    /// encapsulate the condition
    template<class F>
    constexpr auto map(F&& f) const& -> decltype(f(value())) {
        if (*this) return f(value());
        if constexpr (!std::is_void_v<decltype(f(value()))>) return {};
    }
    template<class F>
    constexpr auto map(F&& f) const&& -> decltype(f(value())) {
        if (*this) return f(value());
        if constexpr (!std::is_void_v<decltype(f(value()))>) return {};
    }

    /// default values
    template<class D>
    constexpr auto orValue(D&& d) const& -> decltype(auto) {
        if (*this) return value();
        return std::forward<D>(d);
    }
    template<class D>
    constexpr auto orValue(D&& d) & -> decltype(auto) {
        if (*this) return value();
        return std::forward<D>(d);
    }
    template<class D>
    constexpr auto orValue(D&& d) && -> decltype(std::move(*this).value()) {
        if (*this) return std::move(*this).value();
        return std::forward<D>(d);
    }

    template<class F>
    constexpr auto operator&&(F&& f)
        -> std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::declval<Data>())), bool>, bool> {
        if (*this) return f(value());
        return false;
    }

    constexpr bool operator==(const This& o) const { return data == o.data; }
    constexpr bool operator!=(const This& o) const { return data != o.data; }
};

template<class T>
struct DefaultPacked;

template<class T>
struct DefaultFunc {
    constexpr T operator()() { return {}; }
};

/// convenience overload for default initialized invalid values
template<class T>
class Optional<DefaultPacked<T>> : public Optional<Packed<DefaultFunc<T>>> {
    using Optional<Packed<DefaultFunc<T>>>::Optional;
};

/// bool with map functionality
using OptionalBool = Optional<DefaultPacked<bool>>;

/// pointers are always default packed
template<class T>
class Optional<T*> : public Optional<DefaultPacked<T*>> {
    using Optional<DefaultPacked<T*>>::Optional;
};

} // namespace meta
