#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "scanner/NumberLiteral.h"
#include "scanner/StringLiteral.h"

namespace intrinsic {

template<>
struct TypeOf<scanner::StringLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".String"};
        info.size = sizeof(scanner::StringLiteral);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        // TODO: add .Text property
    }
};

template<>
struct TypeOf<scanner::NumberLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Number"};
        info.size = sizeof(scanner::NumberLiteral);
        info.flags = TypeFlag::CompileTime;
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
        // mod.template type<Identifier>();
        // mod.template type<Operator>();
        mod.template type<scanner::NumberLiteral>();
        mod.template type<scanner::StringLiteral>();
        // mod.template type<scanner::BlockLiteral>();
    }
};

} // namespace intrinsic
