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
using View = text::View;
using OptView = strings::OptionalView;
using Name = strings::String;
using OptName = strings::OptionalString;

struct Node;
using Nodes = std::vector<Node>;
using NodePtr = std::unique_ptr<Node>;

struct Block {
    using This = Block;
    Nodes nodes{};

    bool operator==(const This& o) const { return nodes == o.nodes; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<Block>);

struct ArgumentAssignment {
    using This = ArgumentAssignment;
    instance::ParameterView parameter{};
    Nodes values{};

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
    Nodes nodes{};

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

using StringLiteral = nesting::StringLiteral;
using NumberLiteral = nesting::NumberLiteral;
using OperatorLiteral = nesting::OperatorLiteral;
using IdentifierLiteral = nesting::IdentifierLiteral;
using BlockLiteral = nesting::BlockLiteral;

using NodeVariant = meta::Variant<
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
static_assert(meta::has_move_assignment<NodeVariant>);

// note: this type is needed because we cannot forward a using definition
struct Node : public NodeVariant {
    META_VARIANT_CONSTRUCT(Node, NodeVariant)
};
using OptNode = meta::Optional<Node>;
using NodeView = const Node*;
using OptNodeView = meta::Optional<NodeView>;

static_assert(meta::has_move_assignment<Node>);
static_assert(meta::has_move_assignment<OptNode>);

struct NameTypeValue {
    using This = NameTypeValue;
    OptName name{}; // name might be empty!
    OptNode type{};
    OptNode value{};

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
    OptNodeView type{};
    OptNodeView value{};

    ViewNameTypeValue() = default;
    ViewNameTypeValue(const NameTypeValue& typed)
        : name(typed.name.map([](const auto& n) -> View { return n; }))
        , type(typed.type.map([](const auto& v) -> NodeView { return &v; }))
        , value(typed.value.map([](const auto& v) -> NodeView { return &v; })) {}
    ViewNameTypeValue(const Node& node)
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
    ViewNameTypeValueTuple(const Node& node)
        : tuple({node}) {}
};
static_assert(meta::has_move_assignment<ViewNameTypeValueTuple>);

} // namespace parser
