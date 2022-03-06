#pragma once
#include "parser/Expression.h"

namespace parser {

struct LocalTupleLookup {
    LocalTupleLookup() = default;
    explicit LocalTupleLookup(const NameTypeValueTuple* tuple) noexcept
        : tuple(tuple) {}

    [[nodiscard]] auto byName(View name) const& -> OptNameTypeValueView {
        if (tuple == nullptr) return {};
        for (const auto& ntv : tuple->tuple) {
            if (ntv.name && name.isContentEqual(ntv.name.value())) return &ntv;
        }
        return {};
    }

private:
    const NameTypeValueTuple* tuple{};
};

struct TupleLookup {

    TupleLookup() = default;
    explicit TupleLookup(const NameTypeValueTuple* tuple) noexcept
        : local(tuple) {}
    explicit TupleLookup(const NameTypeValueTuple* tuple, const TupleLookup* parent) noexcept
        : local(tuple)
        , parent(parent) {}

    [[nodiscard]] auto byName(View name) const& -> OptNameTypeValueView {
        auto optNode = local.byName(name);
        if (!optNode && parent != nullptr) return parent->byName(name);
        return optNode;
    }

private:
    LocalTupleLookup local{};
    const TupleLookup* parent{};
};

} // namespace parser
