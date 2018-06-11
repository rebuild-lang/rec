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
        // TODO: add .Text property
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
        // TODO: add API
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
        // TODO: add API
    }
};

template<>
struct TypeOf<parser::expression::IdentifierLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Identifier"};
        info.size = sizeof(parser::block::Token);
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::SingleToken;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO: add API
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
        // mod.template type<Identifier>(); // use Token
        // mod.template type<Operator>();
        mod.template type<parser::expression::NumberLiteral>();
        mod.template type<parser::expression::StringLiteral>();
        mod.template type<parser::expression::BlockLiteral>();
        mod.template type<parser::expression::IdentifierLiteral>();
    }
};

} // namespace intrinsic
