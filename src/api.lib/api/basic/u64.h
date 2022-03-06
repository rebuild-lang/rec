#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "parser/Expression.h"

#include <stdexcept>
#include <string>

namespace api {

using U64 = uint64_t;

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::U64> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".u64"};
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }

    struct Result {
        api::U64 v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"result"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };
    struct Literal {
        parser::NumberLiteral v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"literal"};
            info.side = ParameterSide::Right;
            info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    static void implicitFrom(const Literal& literal, Result& res) {
        auto str = static_cast<std::string>(strings::to_string(literal.v.value.integerPart));
        auto base = static_cast<int>(literal.v.value.radix);
        try {
            res.v = std::stoull(str, nullptr, base);
        }
        catch (const std::invalid_argument&) {
        }
        catch (const std::out_of_range&) {
            // TODO(arBmind): add error
            res.v = 0;
        }
    }

    struct Left {
        api::U64 v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"left"};
            info.side = ParameterSide::Left;
            return info;
        }
    };
    struct Right {
        api::U64 v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"right"};
            info.side = ParameterSide::Right;
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
        mod.function(ptr_to<implicitFrom>, [] {
            auto info = FunctionInfo{};
            info.name = Name{".implicitFrom"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }());
        mod.function(ptr_to<add>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"add"};
            return info;
        }());
        mod.function(ptr_to<sub>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"sub"};
            return info;
        }());
        mod.function(ptr_to<mul>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"mul"};
            return info;
        }());
    }
};

} // namespace intrinsic
