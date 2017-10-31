#pragma once

#include <experimental/coroutine>

namespace meta {

namespace std {
using namespace ::std;
using namespace ::std::experimental;
} // namespace std

template<class T>
struct co_enumerator {
    using element_type = T;
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        T current_m;

        auto get_return_object() { return co_enumerator{handle::from_promise(*this)}; }

        constexpr static auto initial_suspend() { return std::suspend_always{}; }
        constexpr static auto final_suspend() { return std::suspend_always{}; }

        auto yield_value(T value) {
            current_m = std::move(value);
            return std::suspend_always{};
        }
        auto return_void() {}
        auto unhandled_exception() {}
    };

    const T &operator*() const { return coroutine_m.promise().current_m; }
    const T *operator->() const { return &coroutine_m.promise().current_m; }
    T &&move() { return std::move(coroutine_m.promise().current_m); }

    operator bool() const { return coroutine_m && !coroutine_m.done(); }
    bool operator++(int) {
        if (coroutine_m) coroutine_m.resume();
        return *this;
    }

#if !defined(__cpp_coroutines) && !defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#    pragma message("Warning: Lacking Coroutine Support!")
#    define co_yield return
#    define co_return                                                                                                  \
        return {}
    co_enumerator() {}
    co_enumerator(const T &) {}
#endif

private:
    co_enumerator(handle h)
        : coroutine_m(h) {}

private:
    handle coroutine_m;
};

} // namespace meta
