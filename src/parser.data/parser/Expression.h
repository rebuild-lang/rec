#pragma once
#include "Value.h"

#include "instance/Views.h"

#include "nesting/Token.h"

#include "text/Range.h"

#include "meta/Variant.h"

#include <list>
#include <vector>

namespace intrinsic {
struct Context;
}

namespace parser {

using TextRange = text::Range;
using text::View;
using OptView = strings::OptionalView;
using Name = strings::String;
using OptName = strings::OptionalString;

struct Expression;
using Expressions = std::vector<Expression>;
using ExpressionPtr = std::unique_ptr<Expression>;

struct Block {
    using This = Block;
    Expressions expressions{};

    bool operator==(const This& o) const { return expressions == o.expressions; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<Block>);

struct ArgumentAssignment {
    using This = ArgumentAssignment;
    instance::ParameterView parameter{};
    Expressions values{};

    bool operator==(const This& o) const { return parameter == o.parameter && values == o.values; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using ArgumentAssignments = std::vector<ArgumentAssignment>;
static_assert(meta::has_move_assignment<ArgumentAssignment>);

struct Call {
    using This = Call;
    instance::FunctionView function{};
    ArgumentAssignments arguments{};

    bool operator==(const This& o) const { return function == o.function && arguments == o.arguments; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<Call>);

struct IntrinsicCall {
    using This = IntrinsicCall;
    using Exec = void (*)(uint8_t*, intrinsic::Context*);
    Exec exec;

    bool operator==(const This& o) const { return exec == o.exec; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<IntrinsicCall>);

struct ParameterReference {
    using This = ParameterReference;
    instance::ParameterView parameter{};

    bool operator==(const This& o) const { return parameter == o.parameter; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<ParameterReference>);

struct VariableReference {
    using This = VariableReference;
    instance::VariableView variable{};

    bool operator==(const This& o) const { return variable == o.variable; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<VariableReference>);

struct VariableInit {
    using This = VariableInit;
    instance::VariableView variable{};
    Expressions nodes{};

    bool operator==(const This& o) const { return variable == o.variable && nodes == o.nodes; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<VariableInit>);

struct ModuleReference {
    using This = ModuleReference;
    instance::ModuleView module{};

    bool operator==(const This& o) const { return module == o.module; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<ModuleReference>);

struct TypeReference {
    using This = TypeReference;
    TypeView type{};

    bool operator==(const This& o) const { return type == o.type; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<TypeReference>);

struct NameTypeValue;
using NameTypeValueView = const NameTypeValue*;
using NameTypeValueList = std::list<NameTypeValue>;

struct NameTypeValueReference {
    using This = NameTypeValueReference;
    NameTypeValueView nameTypeValue{};

    bool operator==(const This& o) const;
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<NameTypeValueReference>);

struct NameTypeValueTuple {
    using This = NameTypeValueTuple;
    NameTypeValueList tuple{};

    bool operator==(const This& o) const { return tuple == o.tuple; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<NameTypeValueTuple>);

using nesting::BlockLiteral;
using nesting::IdentifierLiteral;
using nesting::NumberLiteral;
using nesting::OperatorLiteral;
using nesting::StringLiteral;

using ExpressionVariant = meta::Variant<
    Block,
    Call,
    IntrinsicCall,
    ParameterReference,
    VariableReference,
    NameTypeValueReference,
    VariableInit,
    ModuleReference,
    TypeReference,
    NameTypeValueTuple,
    Value>;
static_assert(meta::has_move_assignment<ExpressionVariant>);

// note: this type is needed because we cannot forward a using definition
struct Expression : public ExpressionVariant {
    META_VARIANT_CONSTRUCT(Expression, ExpressionVariant)
};
using OptExpression = meta::Optional<Expression>;
using ExpressionView = const Expression*;
using OptExpressionView = meta::Optional<ExpressionView>;

static_assert(meta::has_move_assignment<Expression>);
static_assert(meta::has_move_assignment<OptExpression>);

struct NameTypeValue {
    using This = NameTypeValue;
    OptName name{}; // name might be empty!
    OptExpression type{};
    OptExpression value{};

    auto onlyValue() const { return !name && !type && value; }

    bool operator==(const This& o) const { return name == o.name && type == o.type && value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using OptNameTypeValue = meta::Optional<meta::DefaultPacked<NameTypeValue>>;
using OptNameTypeValueView = meta::Optional<meta::DefaultPacked<NameTypeValueView>>;

static_assert(meta::has_move_assignment<NameTypeValue>);

struct ViewNameTypeValue {
    using This = ViewNameTypeValue;
    OptView name{};
    OptExpressionView type{};
    OptExpressionView value{};

    ViewNameTypeValue() = default;
    ViewNameTypeValue(const NameTypeValue& typed)
        : name(typed.name.map([](const auto& n) -> View { return n; }))
        , type(typed.type.map([](const auto& v) -> ExpressionView { return &v; }))
        , value(typed.value.map([](const auto& v) -> ExpressionView { return &v; })) {}
    ViewNameTypeValue(const Expression& node)
        : value(&node) {}
};
using ViewNameTypeValues = std::vector<ViewNameTypeValue>;

static_assert(meta::has_move_assignment<ViewNameTypeValue>);

struct ViewNameTypeValueTuple {
    using This = ViewNameTypeValueTuple;
    ViewNameTypeValues tuple{};

    ViewNameTypeValueTuple() = default;
    ViewNameTypeValueTuple(const NameTypeValueTuple& typed)
        : tuple(typed.tuple.begin(), typed.tuple.end()) {}
    ViewNameTypeValueTuple(const Expression& node)
        : tuple({node}) {}
};
static_assert(meta::has_move_assignment<ViewNameTypeValueTuple>);

} // namespace parser
