#pragma once
#include "LocalScope.h"
#include "Node.h"

namespace instance {

struct Scope {
    using This = Scope;

    const This* parent{};
    LocalScope locals;

    Scope() = default;
    ~Scope() = default;
    explicit Scope(const This* parent)
        : parent(parent) {}

    // move enabled
    Scope(This&&) = default;
    auto operator=(This &&) -> This& = default;

    // no copy
    Scope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;

public:
    auto operator[](NameView name) const& -> const OptConstNodeView {
        auto optNode = locals[name];
        if (!optNode && parent) return (*parent)[name];
        return optNode;
    }

    auto emplace(Node&& node) & -> OptNodeView { return locals.emplace(std::move(node)); }
};

} // namespace instance
