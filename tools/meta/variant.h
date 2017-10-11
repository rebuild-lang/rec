#pragma once
#include <type_traits>
#include <utility>
#include <variant>

namespace meta {

/// renaming of std::variant
// use is until we find reason to roll our own
template<class... T>
using variant = std::variant<T...>;

using std::visit;

/// check for multiple types in variant
template<class... T, class... V>
bool holds_one_of(const variant<V...> &v) {
    bool sum = false;
    auto x = {(sum = sum || std::holds_alternative<T>(v), 0)...};
    (void)x;
    return sum;
}

namespace details {

template<class V>
struct meta_variant;

template<class... V>
struct meta_variant<variant<V...>> {
    using indices_t = std::make_index_sequence<sizeof...(V)>;

    template<class T>
    constexpr static int type_index_impl(std::index_sequence<>) {
        return -1;
    }

    template<class T, class VC, class... VR, size_t J, size_t... I>
    constexpr static int type_index_impl(std::index_sequence<J, I...>) {
        if constexpr (std::is_same_v<T, VC>) {
            return J;
        }
        else if constexpr (0 < sizeof...(VR)) {
            return type_index_impl<T, VR...>(std::index_sequence<I...>{});
        }
        return -1;
    }

    template<class T>
    constexpr static int type_index() {
        return type_index_impl<T, V...>(indices_t{});
    }

    template<size_t... I>
    constexpr static bool holds_index_impl(int index, std::index_sequence<I...>) {
        bool sum = false;
        auto x = {((sum = sum || (index == I)), 0)...};
        (void)x;
        return sum;
    }

    template<class... T>
    constexpr static bool holds_index(int index) {
        using t_indices = std::index_sequence<type_index<T>()...>;
        return holds_index_impl(index, t_indices{});
    }
};

} // namespace details

/// get index of type T in the variant V without an instance
template<class V, class T>
constexpr const int type_index = details::meta_variant<V>::template type_index<T>();

/// check if variant type V holds any of types T when it contains index
template<class V, class... T>
constexpr bool holds_one_of(int index) {
    return details::meta_variant<V>::template holds_index<T...>(index);
}

} // namespace meta
