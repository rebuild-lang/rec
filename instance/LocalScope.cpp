#include "LocalScope.h"

#include "Node.h"

namespace instance {

auto LocalScope::operator[](const View &name) const & -> const Node * {
    auto it = m.find(name);
    if (it == m.end()) return nullptr;
    return &it->second;
}

bool LocalScope::emplaceImpl(Node &&instance) & {
    const auto &name = nameOf(instance);
    auto[_, success] = m.try_emplace(name, std::move(instance));
    return success;
}

} // namespace instance
