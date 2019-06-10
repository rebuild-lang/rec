#include "LocalScope.h"

#include "Node.h"

namespace instance {

LocalScope::LocalScope() = default;

auto LocalScope::operator[](NameView name) const& -> OptConstNodeView {
    auto it = m.find(name);
    if (it == m.end()) return {};
    return &it->second;
}

auto LocalScope::operator[](NameView name) & -> OptNodeView {
    auto it = m.find(name);
    if (it == m.end()) return {};
    return &it->second;
}

auto LocalScope::emplaceImpl(Node&& node) & -> OptNodeView {
    auto name = nameOf(node);
    if (auto [it, success] = m.try_emplace(name, std::move(node)); success) return &it->second;
    return {};
}

} // namespace instance
