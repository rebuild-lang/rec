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
    struct Promise;
    using Handle = std::coroutine_handle<Promise>;

    struct Promise {
        V v;

        auto get_return_object() { return CoEnumerator{Handle::from_promise(*this)}; }

        constexpr static auto initial_suspend() { return std::suspend_always{}; }
        constexpr static auto final_suspend() { return std::suspend_always{}; }

        auto yield_value(V value) {
            v = std::move(value);
            return std::suspend_always{};
        }
        auto return_void() {}
        auto unhandled_exception() {}
    };

    const V& operator*() const { return handle.promise().v; }
    const V* operator->() const { return &handle.promise().v; }
    V&& move() { return std::move(handle.promise().v); }

    operator bool() const { return handle && !handle.done(); }
    bool operator++(int) {
        if (handle) handle.resume();
        return *this;
    }

#if !defined(__cpp_coroutines) && !defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#    pragma message("Warning: Lacking Coroutine Support!")
#    define co_yield return
#    define co_return                                                                                                  \
        return {}
#endif

    ~CoEnumerator() {
        if (handle) handle.destroy();
    }
    CoEnumerator() = delete;
    CoEnumerator(const CoEnumerator&) = delete;
    CoEnumerator(CoEnumerator&& o) noexcept
        : handle(o.handle) {
        o.handle = {};
    }
    CoEnumerator& operator=(const CoEnumerator&) = delete;
    CoEnumerator& operator=(CoEnumerator&&) = delete;

private:
    CoEnumerator(Handle h)
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
