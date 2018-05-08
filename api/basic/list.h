#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "instance/Type.h"

#include <vector>

namespace api {

struct List {
    using Type = instance::Type;
    using Value = int; // TODO
    using Index = uint64_t;

    List(Type& elementType)
        : elementType(&elementType) {}

    auto length() const -> Index { return m.size() / elementType->size; }
    // auto at(Index i) const -> Value {  }

    // auto append(Value v) const -> List;
    // auto replace(Index i, Value v) const -> List;
    // auto remove(Index i) const -> List;

private:
    Type* elementType{};
    std::vector<uint8_t> m{};
};

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::List> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"list"};
        info.size = sizeof(api::List);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct Result {
        api::List v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };

    struct TypeArgument {
        instance::Type* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"type"};
            info.side = ArgumentSide::Right;
            return info;
        }
    };

    static void construct(TypeArgument type, Result& res) {
        // TODO
        (void)type;
        (void)res;
    }
    static constexpr auto constructInfo() {
        auto info = FunctionInfo{};
        info.name = Name{".construct"};
        info.flags = FunctionFlag::CompileTimeOnly;
        return info;
    }

    using TypeData = instance::Type*;
    static auto eval(TypeArgument type) -> TypeData { return type.v; }

    static auto construct(TypeData typeData) -> api::List { return {*typeData}; }
    static auto destruct(api::List& list) { list.~List(); }

    template<class Module>
    static void module(Module& mod) {
        (void)mod;
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<Length>();
        // mod.function<At>();
    }
};

} // namespace intrinsic
