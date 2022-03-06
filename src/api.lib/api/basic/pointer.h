#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "instance/Type.h"
#include "parser/Value.h"

namespace api {

using instance::Type;
using parser::Value;

struct PointerModule {
    parser::TypeView targetType{};
};
using Pointer = std::intptr_t;

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::Pointer> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"pointer"};
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime; // | TypeFlag::Construct;
        return info;
    }

    struct TargetType {
        parser::TypeView v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"targetType"};
            info.side = ParameterSide::Right;
            return info;
        }
    };
    static auto createModule(TargetType targetType) -> api::PointerModule { return {targetType.v}; }

    struct Self {
        api::Pointer v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"self"};
            info.side = ParameterSide::Right;
            info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct AddressResult {
        uint64_t v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"result"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };
    static void address(const api::PointerModule&, const Self& ptr, AddressResult& res) { res.v = ptr.v; }

    struct TargetResult {
        parser::Value v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"result"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable || ParameterFlag::Transparent;
            return info;
        }
    };
    static void value(const api::PointerModule& module, const Self& ptr, TargetResult& res) {
        res.v = parser::Value(module.targetType);
        module.targetType->cloneFunc(res.v.data(), reinterpret_cast<void*>(ptr.v));
    }

    // void assign(Instance& ptr, std::intptr_t address) const { ptr = address; }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.function(ptr_to<address>, [] {
            auto info = FunctionInfo{};
            info.name = Name{".address"};
            return info;
        }());
        mod.function(ptr_to<value>, [] {
            auto info = FunctionInfo{};
            info.name = Name{".value"};
            return info;
        }());
    }
};

} // namespace intrinsic
