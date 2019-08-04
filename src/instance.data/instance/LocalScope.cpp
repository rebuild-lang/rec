#include "LocalScope.h"

#include "Node.h"

namespace instance {

LocalScope::LocalScope() { new (&m_storage) NodeByName(); }

LocalScope::~LocalScope() { m().~NodeByName(); }

LocalScope::LocalScope(This&& o) { new (&m_storage) NodeByName(std::move(o.m())); }

auto LocalScope::operator=(This&& o) -> This& {
    new (&m_storage) NodeByName(std::move(o.m()));
    return *this;
}

auto LocalScope::operator[](NameView name) const& -> ConstNodeRange {
    auto [b, e] = m().equal_range(name);
    return {b, e};
}

auto LocalScope::operator[](NameView name) & -> NodeRange {
    auto [b, e] = m().equal_range(name);
    return {b, e};
}

auto LocalScope::begin() -> NodeByName::iterator { return m().begin(); }
auto LocalScope::end() -> NodeByName::iterator { return m().end(); }

inline auto LocalScope::m() -> NodeByName& { return *std::launder(reinterpret_cast<NodeByName*>(&m_storage)); }
inline auto LocalScope::m() const -> const NodeByName& {
    return *std::launder(reinterpret_cast<const NodeByName*>(&m_storage));
}

auto LocalScope::emplaceImpl(Node&& node) & -> NodeView {
    auto name = nameOf(node);
    return &m().emplace(name, std::move(node))->second;
}

} // namespace instance
