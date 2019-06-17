#include "LocalScope.h"

#include "Node.h"

namespace instance {

LocalScope::LocalScope() = default;

auto LocalScope::operator[](NameView name) const& -> ConstNodeRange {
    auto [b, e] = m.equal_range(name);
    return {b, e};
}

auto LocalScope::operator[](NameView name) & -> NodeRange {
    auto [b, e] = m.equal_range(name);
    return {b, e};
}

auto LocalScope::emplaceImpl(Node&& node) & -> NodeView {
    auto name = nameOf(node);
    return &m.emplace(name, std::move(node))->second;
}

} // namespace instance
