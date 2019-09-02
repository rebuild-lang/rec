#pragma once
#include "TypeTree.h"
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

class Node;
using Nodes = std::vector<Node>;
using NodePtr = std::unique_ptr<Node>;

struct Block {
    using This = Block;
    Nodes nodes{};

    bool operator==(const This& o) const { return nodes == o.nodes; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct ArgumentAssignment {
    using This = ArgumentAssignment;
    instance::ParameterView parameter{};
    Nodes values{};

    bool operator==(const This& o) const { return parameter == o.parameter && values == o.values; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using ArgumentAssignments = std::vector<ArgumentAssignment>;

struct Call {
    using This = Call;
    instance::FunctionView function{};
    ArgumentAssignments arguments{};

    bool operator==(const This& o) const { return function == o.function && arguments == o.arguments; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct IntrinsicCall {
    using This = IntrinsicCall;
    using Exec = void (*)(uint8_t*, intrinsic::Context*);
    Exec exec;

    bool operator==(const This& o) const { return exec == o.exec; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct ParameterReference {
    using This = ParameterReference;
    instance::ParameterView parameter{};

    bool operator==(const This& o) const { return parameter == o.parameter; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct VariableReference {
    using This = VariableReference;
    instance::VariableView variable{};

    bool operator==(const This& o) const { return variable == o.variable; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct VariableInit {
    using This = VariableInit;
    instance::VariableView variable{};
    Nodes nodes{};

    bool operator==(const This& o) const { return variable == o.variable && nodes == o.nodes; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct ModuleReference {
    using This = ModuleReference;
    instance::ModuleView module{};

    bool operator==(const This& o) const { return module == o.module; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct NameTypeValue;
using NameTypeValueView = const NameTypeValue*;
using NameTypeValueList = std::list<NameTypeValue>;

struct NameTypeValueReference {
    using This = NameTypeValueReference;
    NameTypeValueView nameTypeValue{};

    bool operator==(const This& o) const;
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct NameTypeValueTuple {
    using This = NameTypeValueTuple;
    NameTypeValueList tuple{};

    bool operator==(const This& o) const { return tuple == o.tuple; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

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
    NameTypeValueTuple,
    Value>;

// note: this type is needed because we cannot forward a using definition
class Node : public NodeVariant {
    using This = Node;

public:
    Node() = default;
    //    Node(const Node&) = delete;
    //    Node& operator=(const Node&) = delete;
    //    Node(Node&&) noexcept = default;
    //    Node& operator=(Node&&) noexcept = default;
    META_VARIANT_CONSTRUCT(Node, NodeVariant)
};
using OptNode = meta::Optional<Node>;
using NodeView = const Node*;
using OptNodeView = meta::Optional<NodeView>;

struct NameTypeValue {
    using This = NameTypeValue;
    OptName name{}; // name might be empty!
    OptTypeExpression type{};
    OptNode value{};

    auto onlyValue() const { return !name && !type && value; }

    bool operator==(const This& o) const { return name == o.name && type == o.type && value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using OptNameTypeValue = meta::Optional<meta::DefaultPacked<NameTypeValue>>;
using OptNameTypeValueView = meta::Optional<meta::DefaultPacked<NameTypeValueView>>;

struct ViewNameTypeValue {
    using This = ViewNameTypeValue;
    OptView name{};
    OptTypeExpressionView type{};
    OptNodeView value{};

    ViewNameTypeValue() = default;
    ViewNameTypeValue(const NameTypeValue& typed)
        : name(typed.name.map([](const auto& n) { return n; }))
        , type(typed.type.map([](const auto& t) { return &t; }))
        , value(typed.value.map([](const auto& v) { return &v; })) {}
    ViewNameTypeValue(const Node& node)
        : value(&node) {}
};
using ViewNameTypeValues = std::vector<ViewNameTypeValue>;

struct ViewNameTypeValueTuple {
    using This = ViewNameTypeValueTuple;
    ViewNameTypeValues tuple{};

    ViewNameTypeValueTuple() = default;
    ViewNameTypeValueTuple(const NameTypeValueTuple& typed)
        : tuple(typed.tuple.begin(), typed.tuple.end()) {}
    ViewNameTypeValueTuple(const Node& node)
        : tuple({node}) {}
};

} // namespace parser
