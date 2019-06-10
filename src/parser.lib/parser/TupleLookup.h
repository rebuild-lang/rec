#pragma once
#include "instance/Scope.h"
#include "parser/Tree.h"

namespace parser {

using instance::OptConstNodeView;

struct LocalTupleLookup {
    const NameTypeValueTuple* tuple{};

    auto operator[](View name) const& -> OptNameTypeValueView;
};

template<class Parent>
struct TupleLookup {
    Parent parent{};
    LocalTupleLookup local{};

    auto operator()(View name) const& -> OptConstNodeView {
        auto optNode = local[name];
        if (!optNode) return parent(name);
        return optNode;
    }
};
template<class Parent>
TupleLookup(Parent, const NameTypeValueTuple*)->TupleLookup<Parent>;

} // namespace parser
