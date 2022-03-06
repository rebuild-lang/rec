#pragma once
#include "Entry.h"
#include "LocalScope.h"

namespace instance {

struct Scope;
using ConstScopePtr = std::shared_ptr<const Scope>;

struct Scope {
    using This = Scope;

    LocalScopePtr locals{};
    ConstScopePtr parent{};

    Scope()
        : locals(std::make_shared<LocalScope>()) {}
    ~Scope() = default;
    explicit Scope(LocalScopePtr locals, ConstScopePtr parent)
        : locals(std::move(locals))
        , parent(std::move(parent)) {}

    explicit Scope(ConstScopePtr parent)
        : locals(std::make_shared<LocalScope>())
        , parent(std::move(parent)) {}

    // move enabled
    Scope(This&&) = default;
    Scope& operator=(This&&) = default;

    // no copy
    Scope(const This&) = delete;
    Scope& operator=(const This&) = delete;

    [[nodiscard]] auto byName(NameView name) const& -> ConstEntryRange {
        if (auto range = locals->byName(name); !range.empty()) return range;
        if (parent) return parent->byName(name);
        return {};
    }

    auto emplace(Entry&& entry) & -> void { locals->emplace(std::move(entry)); }
};

} // namespace instance
