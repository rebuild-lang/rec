#pragma once

#include <experimental/coroutine>

namespace meta {

namespace std {
using namespace ::std;
using namespace ::std::experimental;
} // namespace std

template<class V>
struct CoEnumerator {
    using element_type = V;
    using This = CoEnumerator;
    struct Promise;
    using Handle = std::coroutine_handle<Promise>;

    struct Promise {
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
    bool operator++(int) {
        if (handle) handle.resume();
        return static_cast<bool>(*this);
    }
    auto operator++() -> This& {
        if (handle) handle.resume();
        return *this;
    }

    struct End {};
    static auto end() -> End { return {}; }

    auto begin() {
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
            bool operator==(End) const { return !gen; }
            bool operator!=(End) const { return !!gen; }
        };
        return Iterator{++(*this)};
    }

#if !defined(__cpp_coroutines) && !defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#    pragma message("Warning: Lacking Coroutine Support!")
#    define co_yield (void)
#    define co_return (void)
#endif

    ~CoEnumerator() {
        if (handle) handle.destroy();
    }
    CoEnumerator() = delete;
    CoEnumerator(const This&) = delete;
    CoEnumerator(This&& o) noexcept
        : handle(o.handle) {
        o.handle = {};
    }
    This& operator=(const This&) = delete;
    This& operator=(This&&) = delete;

private:
    explicit CoEnumerator(Handle h)
        : handle(h) {}

private:
    Handle handle;
};

} // namespace meta

namespace std::experimental {

template<class T, class... Vs>
struct coroutine_traits<meta::CoEnumerator<T>, Vs...> {
    using promise_type = typename meta::CoEnumerator<T>::Promise;
};

} // namespace std::experimental
