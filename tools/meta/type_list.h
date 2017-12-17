#pragma once

#include <type_traits>
#include <utility>

namespace meta {

template<class T>
struct type_t {};

template<class A, class B>
constexpr bool operator==(type_t<A>, type_t<B>) {
    if constexpr (std::is_same_v<A, B>) {
        return true;
    }
    else
        return false;
}
template<class A, class B>
constexpr bool operator!=(type_t<A> a, type_t<B> b) {
    return !(a == b);
}

template<class... T>
struct type_list_t {
    enum : size_t { npos = sizeof...(T) };
    using indices_t = std::make_index_sequence<sizeof...(T)>;

    template<class C>
    constexpr static size_t index_of(type_t<C> = {}) {
        return index_of_impl<C>(indices_t{});
    }

    template<class C>
    constexpr static size_t contains(type_t<C> = {}) {
        auto result = false;
        auto x = {(type_t<C>{} == type_t<T>{} ? result = true : false)...};
        return result;
    }

    template<class... C>
    constexpr static size_t contains_any(type_t<C>...) {
        auto result = false;
        auto x = {(contains<C>() ? result = true : false)...};
        return result;
    }
    template<class... C>
    constexpr static size_t contains_any() {
        return contains_any(type_t<C>{}...);
    }

    template<size_t index>
    constexpr static auto type_at() {
        if constexpr (0 != sizeof...(T)) {
            return type_at_impl<index, T...>();
        }
        else {
            return type_t<void>{};
        }
    }

private:
    template<class C, size_t... I>
    constexpr static size_t index_of_impl(std::index_sequence<I...>) {
        size_t result = npos;
        (void)std::initializer_list<int>({((std::is_same_v<C, T> ? result = I : 0), 0)...});
        return result;
    }

    template<size_t i, class V, class... W>
    constexpr static auto type_at_impl() {
        if constexpr (0 == i) {
            return type_t<V>{};
        }
        else if constexpr (0 != sizeof...(W)) {
            return type_at<i - 1, W...>();
        }
        else {
            return type_t<void>{};
        }
    }
};

} // namespace meta
