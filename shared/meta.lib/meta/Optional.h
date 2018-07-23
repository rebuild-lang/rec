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
    using Value = T;

    constexpr Optional() = default;
    constexpr Optional(const T& t)
        : m{t} {}
    constexpr Optional(T&& t)
        : m(std::move(t)) {}

    constexpr explicit operator bool() const { return m.has_value(); }
    constexpr auto value() const& -> const Value& { return m.value(); }
    constexpr auto value() & -> Value& { return m.value(); }
    constexpr auto value() && -> Value { return std::move(m).value(); }

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
    constexpr auto orValue(const D& d) const& -> const Value& {
        if (m.has_value()) return m.value();
        return d;
    }
    template<class D>
    constexpr auto orValue(D& d) & -> Value& {
        if (m.has_value()) return m.value();
        return d;
    }
    template<class D>
    constexpr auto orValue(D&& d) && -> Value {
        if (m.has_value()) return std::move(m).value();
        return std::forward<D>(d);
    }

    template<class F>
    constexpr auto operator&&(F&& f) const& //
        -> std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::declval<Value>())), bool>, bool> {

        if (*this) return f(value());
        return false;
    }

    constexpr bool operator==(const This& o) const { return m == o.m; }
    constexpr bool operator!=(const This& o) const { return m != o.m; }
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
    using Value = Data;

    constexpr Optional() = default;
    constexpr Optional(const Data& data)
        : data(data) {}

    constexpr explicit operator bool() const { return !(data == InvalidFunc{}()); }
    constexpr auto value() const& -> const Value& { return data; }
    constexpr auto value() & -> Value& { return data; }
    constexpr auto value() && -> Value { return std::move(data); }

    /// encapsulate the condition
    template<class F>
    constexpr auto map(F&& f) const& -> decltype(f(value())) {
        if (*this) return f(value());
        if constexpr (!std::is_void_v<decltype(f(value()))>) return {};
    }
    template<class F>
    constexpr auto map(F&& f) && -> decltype(f(std::move(*this).value())) {
        if (*this) return f(std::move(*this).value());
        if constexpr (!std::is_void_v<decltype(f(std::move(*this).value()))>) return {};
    }

    /// default values
    template<class D>
    constexpr auto orValue(const D& d) const& -> const Value& {
        if (*this) return value();
        return d;
    }
    template<class D>
    constexpr auto orValue(D& d) & -> Value& {
        if (*this) return value();
        return d;
    }
    template<class D>
    constexpr auto orValue(D&& d) && -> Value {
        if (*this) return std::move(*this).value();
        return std::forward<D>(d);
    }

    template<class F>
    constexpr auto operator&&(F&& f) const& //
        -> std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::declval<Value>())), bool>, bool> {

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

/// references are always packed but api changes
template<class T>
class Optional<T&> {
    using This = Optional;
    using M = Optional<DefaultPacked<T*>>;
    M m{};

public:
    using Value = T;

    constexpr Optional() = default;
    constexpr Optional(T& t)
        : m{&t} {}
    constexpr Optional(std::reference_wrapper<T> t)
        : m{&t.get()} {}
    // constexpr auto operator=(T& t) { m = &t; }

    constexpr explicit operator bool() const { return m ? true : false; }
    constexpr auto value() const& -> const Value& { return *m.value(); }
    constexpr auto value() && -> Value { return *std::move(m).value(); }
    constexpr auto value() & -> Value& { return *m.value(); }

    /// encapsulate the condition
    template<class F>
    constexpr auto map(F&& f) const& -> decltype(f(value())) {
        if (*this) return f(value());
        if constexpr (!std::is_void_v<decltype(f(value()))>) return {};
    }
    template<class F>
    constexpr auto map(F&& f) && -> decltype(f(std::move(*this).value())) {
        if (*this) return f(std::move(*this).value());
        if constexpr (!std::is_void_v<decltype(f(std::move(*this).value()))>) return {};
    }

    /// default values
    template<class D>
    constexpr auto orValue(const D& d) const& -> const Value& {
        if (*this) return value();
        return d;
    }
    template<class D>
    constexpr auto orValue(D&& d) & -> Value {
        if (*this) return value();
        return std::forward<D>(d);
    }
    template<class D>
    constexpr auto orValue(D&& d) && -> Value {
        if (*this) return std::move(*this).value();
        return std::forward<D>(d);
    }

    template<class F>
    constexpr auto operator&&(F&& f) const& //
        -> std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::declval<Value&>())), bool>, bool> {

        if (*this) return f(value());
        return false;
    }

    constexpr bool operator==(const This& o) const { return operator bool() ? value() == o.value() : !o; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace meta
