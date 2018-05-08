#pragma once

#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "strings/String.h"

namespace api {

using String = strings::String;

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::String> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"str"};
        info.size = sizeof(api::String);
        info.flags = TypeFlag::CompileTime; // | TypeFlag::Construct;
        return info;
    }

    template<class Module>
    static void module(Module& mod) {
        (void)mod;
        // TODO
        // mod.function<ImplicitFromLiteral>();
        // mod.function<Length>();
        // mod.function<At>();
        // mod.function<Append>();
    }
};

} // namespace intrinsic
