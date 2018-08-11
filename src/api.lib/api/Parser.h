#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "parser/Tree.h"

namespace intrinsic {

template<>
struct TypeOf<parser::VariableInit> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".VariableInit"};
        info.size = sizeof(parser::VariableInit);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<parser::TypedTuple> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".TypedTuple"};
        info.size = sizeof(parser::TypedTuple);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        // TODO(arBmind): add API
    }
};

struct ParserModule {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{".parser"}; // AST
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template type<parser::VariableInit>();
        mod.template type<parser::TypedTuple>();
    }
};

} // namespace intrinsic
