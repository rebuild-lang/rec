#pragma once
#include "strings/View.h"

#include <map>

namespace instance {

using Name = strings::CompareView;
class Node;

using NodeByName = std::map<Name, Node>;

struct LocalScope {
    using This = LocalScope;
    NodeByName m{};

    LocalScope() = default;
    ~LocalScope() = default;

    // non copyable
    LocalScope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move enabled
    LocalScope(This&&) = default;
    auto operator=(This &&) -> This& = default;

    auto operator[](const Name& name) const& -> const Node*;

    template<class T, class... Ts>
    bool emplace(T&& arg, Ts&&... args) & {
        return (emplaceImpl(std::forward<T>(arg)) & ... & emplaceImpl(std::forward<Ts>(args)));
    }

private:
    bool emplaceImpl(Node&& instance) &;

    // bool replace(old, new)
};

} // namespace instance
