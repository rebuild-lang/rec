#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "scanner/NumberLiteral.h"

namespace intrinsic {

template<>
struct TypeOf<uint64_t> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"u64"};
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }

    struct Result {
        uint64_t v;
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
            return info;
        }
    };
    static void implicitFrom(const Literal& literal, Result& res) {
        // TODO
    }

    struct Left {
        uint64_t v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"left"};
            info.side = ArgumentSide::Left;
            return info;
        }
    };
    struct Right {
        uint64_t v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"right"};
            info.side = ArgumentSide::Right;
            return info;
        }
    };
    static void add(const Left& l, const Right& r, Result& res) {
        res.v = l.v + r.v; //
    }
    static void sub(const Left& l, const Right& r, Result& res) {
        res.v = l.v - r.v; //
    }
    static void mul(const Left& l, const Right& r, Result& res) {
        res.v = l.v * r.v; //
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template function<[] {
            auto info = FunctionInfo{};
            info.name = Name{".implicitFrom"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>(&implicitFrom);
        mod.template function<[] {
            auto info = FunctionInfo{};
            info.name = Name{"add"};
            return info;
        }>(&add);
        mod.template function<[] {
            auto info = FunctionInfo{};
            info.name = Name{"sub"};
            return info;
        }>(&sub);
        mod.template function<[] {
            auto info = FunctionInfo{};
            info.name = Name{"mul"};
            return info;
        }>(&mul);
    }
};

} // namespace intrinsic
