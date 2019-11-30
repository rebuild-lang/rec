#pragma once
#include "Entry.h"
#include "LocalScope.h"

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
    Scope& operator=(This&&) = default;

    // no copy
    Scope(const This&) = delete;
    Scope& operator=(const This&) = delete;

public:
    auto operator[](NameView name) const& -> ConstEntryRange {
        auto range = locals[name];
        if (range.empty() && parent != nullptr) return (*parent)[name];
        return range;
    }

    auto emplace(Entry&& entry) & -> EntryView { return locals.emplace(std::move(entry)); }
};

} // namespace instance
