#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "parser/Expression.h"

namespace intrinsic {

template<>
struct TypeOf<parser::VariableInit> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".VariableInit"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO(arBmind): add API
    }
};

template<>
struct TypeOf<parser::NameTypeValueTuple> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".TypedTuple"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
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
        mod.template type<parser::NameTypeValueTuple>();
    }
};

} // namespace intrinsic
