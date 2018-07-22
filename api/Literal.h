#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "parser/expression/Tree.h"

namespace intrinsic {

template<>
struct TypeOf<parser::expression::StringLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".String"};
        info.size = sizeof(parser::expression::StringLiteral);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::SingleToken;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add .Text property
    }
};

template<>
struct TypeOf<parser::expression::NumberLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Number"};
        info.size = sizeof(parser::expression::NumberLiteral);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::SingleToken;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<parser::expression::BlockLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Block"};
        info.size = sizeof(parser::expression::BlockLiteral);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::SingleToken;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<parser::expression::IdentifierLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Identifier"};
        info.size = sizeof(parser::expression::IdentifierLiteral);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::SingleToken;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<parser::expression::Typed> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Typed"};
        info.size = sizeof(parser::expression::Typed);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::IdTypeValue;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

struct Literal {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{".literal"};
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template type<parser::expression::NumberLiteral>();
        mod.template type<parser::expression::StringLiteral>();
        mod.template type<parser::expression::BlockLiteral>();
        mod.template type<parser::expression::IdentifierLiteral>();
        // mod.template type<OperatorLiteral>();
        mod.template type<parser::expression::Typed>();
    }
};

} // namespace intrinsic
