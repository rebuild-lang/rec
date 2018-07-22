#pragma once
#include "strings/View.h"

#include <map>

namespace instance {

using Name = strings::CompareView;
class Node;
using NodeView = Node*;
using OptNodeView = meta::Optional<NodeView>;
using OptConstNodeView = meta::Optional<const Node*>;

using NodeByName = std::map<Name, Node>;

struct LocalScope {
    using This = LocalScope;
    NodeByName m;

    LocalScope();
    ~LocalScope() = default;

    // non copyable
    LocalScope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move enabled
    LocalScope(This&&) = default;
    auto operator=(This &&) -> This& = default;

    auto operator[](const Name& name) const& -> OptConstNodeView;
    auto operator[](const Name& name) & -> OptNodeView;

    auto emplace(Node&& node) & -> OptNodeView { return emplaceImpl(std::move(node)); }

private:
    auto emplaceImpl(Node&&) & -> OptNodeView;

    // bool replace(old, new)
};

} // namespace instance
