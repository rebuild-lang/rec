#pragma once
#include <initializer_list>
#include <type_traits>

namespace meta {

template<class T>
struct Flags {
    using This = Flags;
    static_assert(std::is_enum_v<T>, "flags only works for enums!");

    using enum_type = T;
    using value_type = std::underlying_type_t<T>;

    constexpr Flags() noexcept = default;
    //    constexpr Flags(const Flags &) noexcept = default;
    //    constexpr Flags(Flags &&) noexcept = default;
    //    ~Flags() = default;

    //    constexpr auto operator=(const Flags &) noexcept -> Flags & = default;
    //    constexpr auto operator=(Flags &&) noexcept -> Flags & = default;

    constexpr void swap(Flags &fl) noexcept { std::swap(v, fl.v); }

    template<class... Args>
    constexpr Flags(T v, Args... args) noexcept
        : Flags(build(v, args...)) {}

    constexpr bool operator==(Flags f) const noexcept { return v == f.v; }
    constexpr bool operator!=(Flags f) const noexcept { return v != f.v; }

    constexpr auto operator|=(Flags f2) noexcept -> Flags & {
        v |= f2.v;
        return *this;
    }
    constexpr auto operator|=(T e2) noexcept -> Flags & {
        v |= static_cast<value_type>(e2);
        return *this;
    }

    constexpr friend auto operator|(Flags f1, Flags f2) noexcept -> Flags { return {f1.v | f2.v}; }

    constexpr auto clear(Flags f = {}) const noexcept -> Flags { return {v & ~f.v}; }
    constexpr bool hasAny(Flags f) const noexcept { return (v & f.v) != 0; }
    constexpr bool hasAll(Flags f) const noexcept { return (v & f.v) == f.v; }

    template<class... Args>
    constexpr bool clear(T v, Args... args) const noexcept {
        return clear(build(v, args...));
    }
    template<class... Args>
    constexpr bool hasAny(T v, Args... args) const noexcept {
        return hasAny(build(v, args...));
    }
    template<class... Args>
    constexpr bool hasAll(T v, Args... args) const noexcept {
        return hasAll(build(v, args...));
    }

private:
    constexpr Flags(value_type v) noexcept
        : v(v) {}

    template<class... Args>
    static constexpr auto build(Args... args) noexcept -> Flags {
        auto val = Flags{0};
        auto x = {((val |= args), 0)...};
        (void)x;
        return val;
    }

private:
    value_type v;
};

} // namespace meta

#define META_FLAGS_OP(T)                                                                                               \
    constexpr auto operator|(T e1, T e2) noexcept->meta::Flags<T> { return meta::Flags<T>(e1) | e2; }
