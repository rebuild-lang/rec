#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "scanner/NumberLiteral.h"

namespace api {

using U64 = uint64_t;

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::U64> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"u64"};
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }

    struct Result {
        api::U64 v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };
    struct Literal {
        scanner::NumberLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void implicitFrom(const Literal& literal, Result& res) {
        // TODO
    }

    struct Left {
        api::U64 v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"left"};
            info.side = ArgumentSide::Left;
            return info;
        }
    };
    struct Right {
        api::U64 v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"right"};
            info.side = ArgumentSide::Right;
            return info;
        }
    };
    static void add(Left l, Right r, Result& res) {
        res.v = l.v + r.v; //
    }
    static void sub(Left l, Right r, Result& res) {
        res.v = l.v - r.v; //
    }
    static void mul(Left l, Right r, Result& res) {
        res.v = l.v * r.v; //
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template function<
            [] {
                auto info = FunctionInfo{};
                info.name = Name{".implicitFrom"};
                info.flags = FunctionFlag::CompileTimeOnly;
                return info;
            },
            asPtr(&implicitFrom)>(&implicitFrom);
        mod.template function<
            [] {
                auto info = FunctionInfo{};
                info.name = Name{"add"};
                return info;
            },
            asPtr(&add)>(&add);
        mod.template function<
            [] {
                auto info = FunctionInfo{};
                info.name = Name{"sub"};
                return info;
            },
            asPtr(&sub)>(&sub);
        mod.template function<
            [] {
                auto info = FunctionInfo{};
                info.name = Name{"mul"};
                return info;
            },
            asPtr(&mul)>(&mul);
    }
};

} // namespace intrinsic
