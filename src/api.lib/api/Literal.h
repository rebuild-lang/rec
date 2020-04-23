#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "parser/Expression.h"

namespace intrinsic {

template<>
struct TypeOf<parser::StringLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".String"};
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
struct TypeOf<parser::ScopedBlockLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Block"};
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
struct TypeOf<parser::NameTypeValue> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".NameTypeValue"};
        info.flags = TypeFlag::CompileTime;
        info.parser = Parser::IdTypeValue;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<nesting::BlockLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".BlockLiteral"};
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
        mod.template type<parser::ScopedBlockLiteral>();
        mod.template type<parser::IdentifierLiteral>();
        mod.template type<parser::NameTypeValue>();
        mod.template type<nesting::BlockLiteral>();
    }
};

} // namespace intrinsic
