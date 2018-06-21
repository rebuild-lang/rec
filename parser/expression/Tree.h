#pragma once
#include "TypeTree.h"

#include "instance/Views.h"

#include "parser/block/Token.h"

#include "scanner/TextRange.h"

#include "meta/Variant.h"

#include <vector>

namespace intrinsic {
struct Context;
}

namespace parser::expression {

using TextRange = scanner::TextRange;
using View = scanner::View;
using Name = strings::String;

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
    using Exec = void (*)(uint8_t*, intrinsic::Context*);
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

struct Named;
using NamedVec = std::vector<Named>;
struct NamedTuple {
    using This = NamedTuple;
    NamedVec tuple{};

    bool operator==(const This& o) const { return tuple == o.tuple; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

struct Typed;
using TypedVec = std::vector<Typed>;

struct TypedTuple {
    using This = TypedTuple;
    TypedVec tuple{};

    bool operator==(const This& o) const { return tuple == o.tuple; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

template<class TokenType>
struct TokenLiteral {
    TokenType token{};
    TextRange range{};

    using This = TokenLiteral;
    bool operator==(const This& o) const { return token == o.token; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using StringLiteral = TokenLiteral<block::StringLiteral>;
using NumberLiteral = TokenLiteral<block::NumberLiteral>;
using OperatorLiteral = TokenLiteral<block::OperatorLiteral>;
using IdentifierLiteral = TokenLiteral<block::IdentifierLiteral>;
using BlockLiteral = TokenLiteral<block::BlockLiteral>;

using NodeVariant = meta::Variant<
    Block,
    Call,
    IntrinsicCall,
    ArgumentReference,
    VariableReference,
    VariableInit,
    ModuleReference,
    NamedTuple,
    TypedTuple,
    StringLiteral,
    NumberLiteral,
    OperatorLiteral,
    IdentifierLiteral,
    BlockLiteral>;

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

struct Typed {
    using This = Typed;
    Name name{};
    OptTypeExpression type{};
    OptNode value{};

    bool operator==(const This& o) const { return name == o.name && type == o.type && value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

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
