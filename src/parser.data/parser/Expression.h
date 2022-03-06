#pragma once
#include "Value.h"

#include "instance/Views.h"

#include "nesting/Token.h"

#include "text/Range.h"

#include "meta/Variant.h"

#include <list>
#include <vector>

namespace intrinsic {
struct ContextInterface;
}

namespace parser {

using TextRange = text::Range;
using text::View;
using OptView = strings::OptionalView;
using Name = strings::String;
using OptName = strings::OptionalString;

using nesting::BlockLiteral;
using nesting::NumberLiteral;
using nesting::StringLiteral;

/// expressions on the block level
struct BlockExpr;
using VecOfBlockExpr = std::vector<BlockExpr>;

/// represents a parsed block
/// - may contain partially parsed expressions
struct Block {
    using This = Block;
    VecOfBlockExpr expressions{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<Block>);

/// triple of name :type =value
/// - we take references by pointer to this
/// - so we have to store a std::list
struct NameTypeValue;
using NameTypeValueView = const NameTypeValue*;
using ListOfNameTypeValue = std::list<NameTypeValue>; // TODO: use std::vector<std::unique_ptr<>>

/// local reference in the same set
/// eg. fn foo(a :type, b: a)
struct NameTypeValueReference {
    using This = NameTypeValueReference;
    NameTypeValueView nameTypeValue{};

    bool operator==(const This& o) const;
    bool operator!=(const This& o) const { return !(*this == o); }
};
static_assert(meta::has_move_assignment<NameTypeValueReference>);

/// reference to local or global variable
struct VariableReference {
    using This = VariableReference;
    instance::VariableView variable{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<VariableReference>);

/// direct reference to a type
struct TypeReference {
    using This = TypeReference;
    TypeView type{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<TypeReference>);

/// Expression that eventually turns into a value
struct ValueExpr;
using VecOfValueExpr = std::vector<ValueExpr>;
using PtrToValueExpr = std::unique_ptr<ValueExpr>;

/// assignment to a parameter of a function for a call
struct ArgumentAssignment {
    using This = ArgumentAssignment;
    instance::ParameterView parameter{};
    VecOfValueExpr values{};

    bool operator==(const This& o) const noexcept = default;
};
using ArgumentAssignments = std::vector<ArgumentAssignment>;
static_assert(meta::has_move_assignment<ArgumentAssignment>);

/// direct call to a function
/// TODO: allow call through variable or NTVRef
struct Call {
    using This = Call;
    instance::FunctionView function{};
    ArgumentAssignments arguments{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<Call>);

/// if we cannot parse any further before knowing types we store everything as partially parsed
struct PartiallyParsed;
using PartiallyParsedView = const PartiallyParsed*;
using VecOfPartiallyParsed = std::vector<PartiallyParsed>;

/// expression used on for types
constexpr auto type_expr_pack = //
    meta::type_pack<
        NameTypeValueReference, // of type Type
        VariableReference, // of type Type
        TypeReference, // direct
        Value, // result of a Call of type Type
        Call, // not executed call (depends on parameters)
        VecOfPartiallyParsed // not fully parsed tokens
        >;
using TypeExprVariant = meta::ApplyPack<meta::Variant, decltype(type_expr_pack)>;
struct TypeExpr : TypeExprVariant {
    META_VARIANT_CONSTRUCT(TypeExpr, TypeExprVariant)
};
using OptTypeExpr = meta::Optional<TypeExpr>;
using TypeExprView = const TypeExpr*;
using OptTypeExprView = meta::Optional<TypeExprView>;

/// direct reference to a module
struct ModuleReference {
    using This = ModuleReference;
    instance::ModuleView module{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<ModuleReference>);

// TODO: FunctionReference

/// expression that initializes variables and submodules of a module
struct InitExpr;
using VecOfInitExpr = std::vector<InitExpr>;

/// paritial initializer for a module
struct ModuleInit {
    using This = ModuleInit;
    instance::ModuleView module{};
    VecOfInitExpr nodes{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<ModuleInit>);

/// initializer for a variable
struct VariableInit {
    using This = VariableInit;
    instance::VariableView variable{};
    VecOfValueExpr nodes{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<VariableInit>);

constexpr auto init_expr_pack = meta::type_pack<
    ModuleInit,
    VariableInit,
    VecOfPartiallyParsed //
    >;
using InitExprVariant = meta::ApplyPack<meta::Variant, decltype(init_expr_pack)>;
struct InitExpr : InitExprVariant {
    META_VARIANT_CONSTRUCT(InitExpr, InitExprVariant)
};

/// wrapper for ListOfNameTypeValue
// TODO: remove if we do not add functionality here
struct NameTypeValueTuple {
    using This = NameTypeValueTuple;
    ListOfNameTypeValue tuple{};

    bool operator==(const This& o) const noexcept = default;
};
static_assert(meta::has_move_assignment<NameTypeValueTuple>);

/// when passing BlockLiteral to functions we might need the Scope from the block definition site.
// so far we use the ParserScope - but we get into troubles with delayed parsing either way
struct ScopedBlockLiteral {
    using This = ScopedBlockLiteral;
    nesting::BlockLiteral block;
    // instance::ScopePtr scope; // parser has no access to scope?

    bool operator==(const This& o) const noexcept = default;
};

using nesting::IdentifierLiteral;

constexpr auto value_expr_pack = //
    type_expr_pack + ///< everything that works for a type
    meta::type_pack<
        ModuleReference, ///< direct reference to a module
        // FunctionReference,
        Block, ///< nested parsed block
        NameTypeValueTuple ///< bunch of values
        >;
using ValueExprVariant = meta::ApplyPack<meta::Variant, decltype(value_expr_pack)>;
struct ValueExpr : public ValueExprVariant {
    META_VARIANT_CONSTRUCT(ValueExpr, ValueExprVariant)
};
using OptValueExpr = meta::Optional<ValueExpr>;
using ValueExprView = const ValueExpr*;
using OptValueExprView = meta::Optional<ValueExprView>;

static_assert(meta::has_move_assignment<ValueExpr>);
static_assert(meta::has_move_assignment<OptValueExpr>);

constexpr auto block_expr_pack = //
    value_expr_pack + ///
    meta::type_pack<
        ModuleInit, ///< initialize local modules
        VariableInit ///< initialize local variables
        >;
using BlockExprVariant = meta::ApplyPack<meta::Variant, decltype(block_expr_pack)>;
struct BlockExpr : public BlockExprVariant {
    META_VARIANT_CONSTRUCT(BlockExpr, BlockExprVariant)
};
using OptBlockExpr = meta::Optional<BlockExpr>;

constexpr auto partial_parsed_pack = //
    value_expr_pack + meta::type_pack<IdentifierLiteral>;
using PartiallyParsedVariant = meta::ApplyPack<meta::Variant, decltype(partial_parsed_pack)>;
static_assert(meta::has_move_assignment<PartiallyParsedVariant>);

struct PartiallyParsed : public PartiallyParsedVariant {
    META_VARIANT_CONSTRUCT(PartiallyParsed, PartiallyParsedVariant)
};
static_assert(meta::has_move_assignment<PartiallyParsed>);

struct NameTypeValue {
    using This = NameTypeValue;
    OptName name{}; // name might be empty!
    OptTypeExpr type{};
    OptValueExpr value{};

    auto onlyValue() const { return !name && !type && value; }

    bool operator==(const This& o) const noexcept = default;
};
using OptNameTypeValue = meta::Optional<meta::DefaultPacked<NameTypeValue>>;
using OptNameTypeValueView = meta::Optional<meta::DefaultPacked<NameTypeValueView>>;

static_assert(meta::has_move_assignment<NameTypeValue>);

} // namespace parser
