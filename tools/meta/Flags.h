#pragma once
#include <initializer_list>
#include <type_traits>

namespace meta {

#define META_FLAGS_OP(Flags)                                                                                           \
    constexpr auto operator|(Flags::Enum e1, Flags::Enum e2) noexcept->Flags { return Flags(e1, e2); }

template<class T>
struct Flags {
    using This = Flags;
    static_assert(std::is_enum_v<T>, "Flags only works for enums!");

    using Enum = T;
    using Value = std::underlying_type_t<T>;

    template<class... Args>
    constexpr Flags(Enum v, Args... args) noexcept
        : Flags(build(v, args...)) {}

    constexpr Flags() noexcept = default;
    constexpr Flags(const This&) noexcept = default;
    constexpr Flags(This&&) noexcept = default;
    ~Flags() = default;
    constexpr auto operator=(const This&) noexcept -> This& = default;
    constexpr auto operator=(This&&) noexcept -> This& = default;

    constexpr auto operator=(Enum e) noexcept -> This& {
        v = static_cast<Value>(e);
        return *this;
    }

    constexpr void swap(This& fl) noexcept { std::swap(v, fl.v); }

    constexpr bool operator==(This f) const noexcept { return v == f.v; }
    constexpr bool operator!=(This f) const noexcept { return v != f.v; }

    constexpr bool operator[](Enum t) const noexcept { return (v & t) != 0; }

    constexpr bool all(This f) const noexcept { return (v & f.v) == f.v; }
    template<class... Args>
    constexpr bool all(Enum b, Args... args) const noexcept {
        return all(build(b, args...));
    }

    constexpr bool any(This f) const noexcept { return (v & f.v) != 0; }
    template<class... Args>
    constexpr bool any(Enum b, Args... args) const noexcept {
        return any(build(b, args...));
    }

    constexpr bool none() const noexcept { return v == 0; }
    constexpr bool none(This f) const noexcept { return (v & f.v) == 0; }
    template<class... Args>
    constexpr bool none(Enum b, Args... args) const noexcept {
        return none(build(b, args...));
    }

    constexpr static auto resetAll() noexcept -> This { return Flags{}; }

    constexpr auto set(This b) const noexcept -> This { return This{v | b.v}; }
    template<class... Args>
    constexpr auto set(Enum b, Args... args) const noexcept -> This {
        return set(build(b, args...));
    }

    constexpr auto reset(This b) const noexcept -> This { return This{v & ~b.v}; }
    template<class... Args>
    constexpr auto reset(Enum b, Args... args) const noexcept -> This {
        return reset(build(b, args...));
    }

    constexpr auto flip(This b) const noexcept -> This { return This{v ^ b.v}; }
    template<class... Args>
    constexpr auto flip(Enum b, Args... args) const noexcept -> This {
        return flip(build(b, args...));
    }

    constexpr auto mask(This b) const noexcept -> This { return This{v & b.v}; }
    template<class... Args>
    constexpr auto mask(Enum b, Args... args) const noexcept -> This {
        return mask(build(b, args...));
    }

    template<class B>
    constexpr auto operator|(B b) const noexcept -> This {
        return set(b);
    }
    template<class B>
    constexpr auto operator&(B b) const noexcept -> This {
        return mask(b);
    }
    template<class B>
    constexpr auto operator^(B b) const noexcept -> This {
        return flip(b);
    }

    template<class B>
    constexpr auto operator|=(B b) noexcept -> This& {
        return *this = set(b);
    }
    template<class B>
    constexpr auto operator&=(B b) noexcept -> This& {
        return *this = mask(b);
    }
    template<class B>
    constexpr auto operator^=(B b) noexcept -> This& {
        return *this = flip(b);
    }

    template<class F>
    constexpr void each_set(F&& f) const noexcept {
        auto vt = Value{};
        while (true) {
            const auto t = static_cast<Enum>(1 << vt);
            const auto ft = Flags{t};
            if (ft.v > v) break;
            if (any(ft)) f(t);
            vt++;
        }
    }

private:
    constexpr Flags(Value v) noexcept
        : v(v) {}

    template<class... Args>
    static constexpr auto build(Args... args) noexcept -> Flags {
        return Flags{(... | static_cast<Value>(args))};
    }

private:
    Value v{};
};

template<class Out, class T>
auto operator<<(Out& out, Flags<T> f)
    -> std::enable_if_t<std::is_same_v<decltype(out << std::declval<T>()), decltype(out)>, decltype(out)> //
{
    using flags_type = Flags<T>;
    if (f == flags_type{}) return out << "<None>";
    f.each_set([&, first = true](T t) mutable {
        if (!first)
            out << " | ";
        else
            first = false;
        out << t;
    });
    return out;
}

} // namespace meta
