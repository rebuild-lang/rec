#pragma once
#include "parser/Tree.h"

namespace parser {

struct LocalTupleLookup {
    const NameTypeValueTuple* tuple{};

    auto operator[](View name) const& -> OptNameTypeValueView {
        for (auto& ntv : tuple->tuple) {
            if (ntv.name && name.isContentEqual(ntv.name.value())) return &ntv;
        }
        return {};
    }
};

struct TupleLookup {
    TupleLookup* parent{};
    LocalTupleLookup tuple{};

    auto operator[](View name) const& -> OptNameTypeValueView {
        auto optNode = tuple[name];
        if (!optNode && parent != nullptr) return (*parent)[name];
        return optNode;
    }
};

} // namespace parser
