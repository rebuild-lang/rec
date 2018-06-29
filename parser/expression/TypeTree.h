#pragma once

#include "tools/meta/Optional.h"
#include "tools/meta/Variant.h"

#include <memory>

namespace instance {
struct Type;
} // namespace instance

namespace parser::expression {

using TypeView = const instance::Type*;

struct TypeExpression;

struct Auto {
    using This = Auto;
    bool operator==(const This&) const { return true; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct Pointer {
    std::shared_ptr<TypeExpression> target;

    using This = Pointer;
    bool operator==(const This& o) const { return target == o.target; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct Array {
    size_t count;
    std::shared_ptr<TypeExpression> element;

    using This = Array;
    bool operator==(const This& o) const { return count == o.count && element == o.element; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct TypeInstance {
    TypeView concrete;

    using This = TypeInstance;
    bool operator==(const This& o) const { return concrete == o.concrete; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

using TypeExpressionVariant = meta::Variant<
    Auto,
    Array,
    Pointer,
    TypeInstance
    // TODO: Call?
    // TODO: TypedTuple?
    >;

struct TypeExpression : TypeExpressionVariant {
    using This = TypeExpression;

public:
    META_VARIANT_CONSTRUCT(TypeExpression, TypeExpressionVariant)
    TypeExpression()
        : TypeExpressionVariant(Auto{}) {}
};
using OptTypeExpression = meta::Optional<TypeExpression>;

} // namespace parser::expression
