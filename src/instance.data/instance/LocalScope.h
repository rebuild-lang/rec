#pragma once
#include "strings/View.h"

#include <map>

namespace instance {

using Name = strings::String;
using NameView = strings::View;

class Node;
using NodeView = Node*;
using OptNodeView = meta::Optional<NodeView>;
using OptConstNodeView = meta::Optional<const Node*>;

using NodeByName = std::multimap<NameView, Node>;
template<class it>
struct Range {
    it _begin;
    it _end;

    bool empty() const { return _begin == _end; }
    bool single() const { return _begin != _end && std::next(_begin) == _end; }
    auto frontValue() const -> auto& { return _begin->second; }
    it begin() const { return _begin; }
    it end() const { return _end; }
};
using NodeRange = Range<NodeByName::iterator>;
using ConstNodeRange = Range<NodeByName::const_iterator>;

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

    auto operator[](NameView name) const& -> ConstNodeRange;
    auto operator[](NameView name) & -> NodeRange;

    auto emplace(Node&& node) & -> NodeView { return emplaceImpl(std::move(node)); }

private:
    auto emplaceImpl(Node&&) & -> NodeView;

    // bool replace(old, new)
};

} // namespace instance
