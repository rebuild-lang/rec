#pragma once
#include "basic/flags.h"
#include "basic/list.h"
#include "basic/str.h"
#include "basic/u64.h"

#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

namespace intrinsic {

struct Basic {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{".basic"};
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        // mod.template type<bool>();
        mod.template type<api::U64>();
        // mod.template type<api::F64>();
        mod.template type<api::String>();
        // mod.template type<api::Rope>();
        mod.template type<api::Flags>();
        // mod.template type<api::Enum>();
        // mod.template type<api::Variant>();
        // mod.template type<api::List>();
        // mod.template type<api::Map>();
    }
};

} // namespace intrinsic
