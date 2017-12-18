#pragma once
#include "strings/View.h"

#include <map>

namespace instance {

using View = strings::View;
class Node;

using NodeByName = std::map<View, Node>;

struct LocalScope {
    using This = LocalScope;
    NodeByName m{};

    LocalScope() = default;
    ~LocalScope() = default;

    // non copyable
    LocalScope(const This &) = delete;
    auto operator=(const This &) -> This & = delete;
    // move enabled
    LocalScope(This &&) = default;
    auto operator=(This &&) -> This & = default;

    auto operator[](const View &name) const & -> const Node *;

    template<class T, class... Ts>
    bool emplace(T &&arg, Ts &&... args) & {
        bool success = emplaceImpl(std::forward<T>(arg));
        if constexpr (0 != sizeof...(Ts)) {
            success = (emplaceImpl(std::forward<Ts>(args)) & ...);
        }
        return success;
    }

private:
    bool emplaceImpl(Node &&instance) &;

    // bool replace(old, new)
};

} // namespace instance
