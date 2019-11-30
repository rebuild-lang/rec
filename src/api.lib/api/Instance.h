#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "instance/Module.h"
#include "instance/Scope.h"
#include "instance/Type.h"

#include "nesting/Token.h"
#include "scanner/Token.h"

namespace intrinsic {

template<>
struct TypeOf<instance::Module*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Module"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {}
};

template<>
struct TypeOf<instance::Type*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Type"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    //    struct Self {
    //        instance::Type v;
    //        static constexpr auto info() {
    //            auto info = ParameterInfo{};
    //            info.name = Name{"self"};
    //            info.side = ParameterSide::Left;
    //            return info;
    //        }
    //    };
    //    struct Result {
    //        instance::TypeFlags v;
    //        static constexpr auto info() {
    //            auto info = ParameterInfo{};
    //            info.name = Name{"result"};
    //            info.side = ParameterSide::Result;
    //            info.flags = ParameterFlag::Assignable;
    //            return info;
    //        }
    //    };
    //    static void readFlags(const Self& self, Result& res) { //
    //        res.v = self.v.flags;
    //    }

    template<class Module>
    static constexpr void module(Module&) {
        // mod.function<ReadName>();
        // mod.function<ReadParent>();
        //        mod.template function<&readFlags,
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".flags"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            }>();
        // mod.function<ReadSize>();
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<ImplicitFrom>();
        // mod.function<Instances>(); // ???
    }
};

template<>
struct TypeOf<instance::Function*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Function"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {}
};

struct Instance {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{"instance"};
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template type<instance::Module*>();
        mod.template type<instance::Type*>();
        mod.template type<instance::Function*>();
        // mod.template type<instance::Variable>();
        // mod.template type<instance::Parameter>();
    }
};

} // namespace intrinsic
