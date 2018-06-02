#include "LocalScope.h"

#include "Node.h"

namespace instance {

LocalScope::LocalScope() = default;

auto LocalScope::operator[](const Name& name) const& -> const Node* {
    auto it = m.find(name);
    if (it == m.end()) return nullptr;
    return &it->second;
}

auto LocalScope::operator[](const Name& name) & -> Node* {
    auto it = m.find(name);
    if (it == m.end()) return nullptr;
    return &it->second;
}

bool LocalScope::emplaceImpl(Node&& instance) & {
    auto name = nameOf(instance);
    auto [_, success] = m.try_emplace(name, std::move(instance));
    return success;
}

} // namespace instance
