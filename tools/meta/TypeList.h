#pragma once

#include <type_traits>
#include <utility>

namespace meta {

template<class T>
struct Type {};

template<class A, class B>
constexpr bool operator==(Type<A>, Type<B>) noexcept {
    if constexpr (std::is_same_v<A, B>) {
        return true;
    }
    else
        return false;
}
template<class A, class B>
constexpr bool operator!=(Type<A> a, Type<B> b) noexcept {
    return !(a == b);
}

template<class... T>
struct TypeList {
    enum : size_t { npos = sizeof...(T) };
    using Indices = std::make_index_sequence<sizeof...(T)>;

    template<class C>
    constexpr static size_t indexOf(Type<C> = {}) {
        return indexOfImpl<C>(Indices{});
    }

    template<class C>
    constexpr static size_t contains(Type<C> = {}) {
        auto result = false;
        auto x = {(Type<C>{} == Type<T>{} ? result = true : false)...};
        return result;
    }

    template<class... C>
    constexpr static size_t containsAny(Type<C>...) {
        auto result = false;
        auto x = {(contains<C>() ? result = true : false)...};
        return result;
    }
    template<class... C>
    constexpr static size_t containsAny() {
        return containsAny(Type<C>{}...);
    }

    template<size_t index>
    constexpr static auto typeAt() {
        if constexpr (0 != sizeof...(T)) {
            return typeAtImpl<index, T...>();
        }
        else {
            return Type<void>{};
        }
    }

private:
    template<class C, size_t... I>
    constexpr static size_t indexOfImpl(std::index_sequence<I...>) {
        size_t result = npos;
        (void)std::initializer_list<int>({((std::is_same_v<C, T> ? result = I : 0), 0)...});
        return result;
    }

    template<size_t i, class V, class... W>
    constexpr static auto typeAtImpl() {
        if constexpr (0 == i) {
            return Type<V>{};
        }
        else if constexpr (0 != sizeof...(W)) {
            return typeAt<i - 1, W...>();
        }
        else {
            return Type<void>{};
        }
    }
};

} // namespace meta
