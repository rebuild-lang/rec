#pragma once
#include "Type.h"

#include "meta/Variant.h"

#include <memory>

namespace instance {
namespace type {

struct Expression;

struct Auto {};

struct Pointer {
    std::shared_ptr<Expression> target;
};

struct Array {
    size_t count; // TODO: store expression
    std::shared_ptr<Expression> element;
};

struct Instance {
    TypeView concrete;
};

using ExpressionVariant = meta::Variant<Auto, Array, Instance, Pointer>;

struct Expression : ExpressionVariant {
    using This = Expression;

public:
    META_VARIANT_CONSTRUCT(Expression, ExpressionVariant)
    Expression()
        : Expression(Auto{}) {}
};

} // namespace type
} // namespace instance
