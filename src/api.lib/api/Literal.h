#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "parser/Tree.h"

namespace intrinsic {

template<>
struct TypeOf<parser::StringLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".String"};
        info.size = sizeof(parser::StringLiteral);
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
struct TypeOf<parser::NumberLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Number"};
        info.size = sizeof(parser::NumberLiteral);
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
struct TypeOf<parser::BlockLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Block"};
        info.size = sizeof(parser::BlockLiteral);
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
struct TypeOf<parser::IdentifierLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Identifier"};
        info.size = sizeof(parser::IdentifierLiteral);
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
struct TypeOf<parser::Typed> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Typed"};
        info.size = sizeof(parser::Typed);
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
        mod.template type<parser::NumberLiteral>();
        mod.template type<parser::StringLiteral>();
        mod.template type<parser::BlockLiteral>();
        mod.template type<parser::IdentifierLiteral>();
        // mod.template type<OperatorLiteral>();
        mod.template type<parser::Typed>();
    }
};

} // namespace intrinsic
