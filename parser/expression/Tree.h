#pragma once
#include "instance/Views.h"

#include "parser/block/Token.h"

#include "scanner/TextRange.h"

#include "meta/Variant.h"

#include <vector>

namespace parser::expression {

using TextRange = scanner::TextRange;
using View = scanner::View;
using Name = strings::String;

class Node;
using Nodes = std::vector<Node>;

struct Named;
using NamedVec = std::vector<Named>;

using StringLiteral = block::StringLiteral;
using NumberLiteral = block::NumberLiteral;
using OperatorLiteral = block::OperatorLiteral;
using IdentifierLiteral = block::IdentifierLiteral;
using BlockLiteral = block::BlockLiteral;

struct Block {
    using This = Block;
    Nodes nodes{};

    bool operator==(const This& o) const { return nodes == o.nodes; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct ArgumentAssignment {
    using This = ArgumentAssignment;
    instance::ArgumentView argument{};
    Nodes values{};

    bool operator==(const This& o) const { return argument == o.argument && values == o.values; }
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
    using Exec = void (*)(uint8_t*);
    Exec exec;

    bool operator==(const This& o) const { return exec == o.exec; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct ArgumentReference {
    using This = ArgumentReference;
    instance::ArgumentView argument{};

    bool operator==(const This& o) const { return argument == o.argument; }
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

struct NamedTuple {
    using This = NamedTuple;
    NamedVec tuple{};

    bool operator==(const This& o) const { return tuple == o.tuple; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

using LiteralVariant = meta::Variant<StringLiteral, NumberLiteral, OperatorLiteral, IdentifierLiteral, BlockLiteral>;

struct Literal {
    using This = Literal;
    LiteralVariant value{};
    TextRange range{};

    bool operator==(const This& o) const { return value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

using NodeVariant = meta::Variant<
    Block,
    Call,
    IntrinsicCall,
    ArgumentReference,
    VariableReference,
    VariableInit,
    ModuleReference,
    NamedTuple,
    Literal>;

// note: this type is needed because we cannot forward a using definition
class Node : public NodeVariant {
    using This = Node;

public:
    Node() = default;
    META_VARIANT_CONSTRUCT(Node, NodeVariant)
};
using OptNode = meta::Optional<Node>;
using NodeView = const Node*;

struct Named {
    using This = Named;
    Name name{}; // name might be empty!
    Node node{};

    Named() = default;
    Named(Name&& name, Node&& node)
        : name(std::move(name))
        , node(std::move(node)) {}

    bool operator==(const This& o) const { return name == o.name && node == o.node; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using OptNamed = meta::Optional<Named>;

struct NamedNodeView {
    using This = NamedNodeView;
    View name{};
    NodeView node{};

    NamedNodeView() = default;
    NamedNodeView(const Named& named)
        : name(named.name)
        , node(&named.node) {}
    NamedNodeView(const Node& node)
        : node(&node) {}
};
using NamedNodeViews = std::vector<NamedNodeView>;

struct NamedTupleView {
    using This = NamedTupleView;
    NamedNodeViews tuple{};

    NamedTupleView() = default;
    NamedTupleView(const NamedTuple& named)
        : tuple(named.tuple.begin(), named.tuple.end()) {}
    NamedTupleView(const Node& node)
        : tuple({node}) {}
};

} // namespace parser::expression
