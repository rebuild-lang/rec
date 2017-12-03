#include "local_scope.h"

#include "node.h"

namespace instance {

auto local_scope_t::operator[](const view_t &name) const & -> const node_t * {
    auto it = names.find(name);
    if (it == names.end()) return nullptr;
    return &it->second;
}

bool local_scope_t::emplace_impl(node_t &&instance) & {
    const auto &name = name_of(instance);
    auto [_, success] = names.try_emplace(name, std::move(instance));
    return success;
}

} // namespace instance
