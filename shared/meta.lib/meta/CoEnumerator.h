#pragma once
#include "CoRoutine.h"

#include <iterator>

namespace meta {

template<class V>
struct CoEnumerator {
    using element_type = V;
    using This = CoEnumerator;
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        V v;

        auto get_return_object() noexcept { return CoEnumerator{Handle::from_promise(*this)}; }

        constexpr static auto initial_suspend() noexcept { return std::suspend_always{}; }
        constexpr static auto final_suspend() noexcept { return std::suspend_always{}; }

        auto yield_value(V value) noexcept {
            v = std::move(value);
            return std::suspend_always{};
        }
        auto return_void() noexcept {}
        auto unhandled_exception() noexcept {}
    };

    auto operator*() const noexcept -> const V& { return handle.promise().v; }
    auto operator-> () const noexcept -> const V* { return &handle.promise().v; }
    auto move() -> V { return std::move(handle.promise().v); }

    explicit operator bool() const { return handle && !handle.done(); }
    auto operator++(int) -> bool {
        if (handle) handle.resume();
        return static_cast<bool>(*this);
    }
    auto operator++() -> This& {
        if (handle) handle.resume();
        return *this;
    }

    struct End {};
    static auto end() -> End { return {}; }

    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = V;
        using reference = V const&;
        using pointer = V const*;

        CoEnumerator& gen;

        auto operator*() const -> const V& { return *gen; }
        auto operator++() -> Iterator& {
            ++gen;
            return *this;
        }
        auto operator==(End) const -> bool { return !gen; }
        auto operator!=(End) const -> bool { return !!gen; }
    };
    auto begin() -> Iterator { return Iterator{++(*this)}; }

    ~CoEnumerator() {
        if (handle) handle.destroy();
    }
    CoEnumerator() = delete;
    CoEnumerator(const This&) = delete;
    CoEnumerator(This&& o) noexcept
        : handle(o.handle) {
        o.handle = {};
    }
    auto operator=(const This&) -> This& = delete;
    auto operator=(This&&) -> This& = delete;

private:
    explicit CoEnumerator(Handle h)
        : handle(h) {}

private:
    Handle handle;
};

} // namespace meta
