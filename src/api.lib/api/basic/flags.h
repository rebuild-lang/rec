#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Type.h"

#include "vector"

namespace api {

struct Flags {
    Flags(std::vector<uint64_t> ids)
        : ids(ids) {}

private:
    std::vector<uint64_t> ids{};
    // uint64_t v{};
};

} // namespace api

namespace intrinsic {

template<>
struct TypeOf<api::Flags> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"flags"};
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct IdentifierLiterals {
        std::vector<uint64_t> v; // TODO(arBmind): parser::Identifier
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"ids"};
            info.side = ParameterSide::Right;
            info.flags = ParameterFlag::Unrolled | ParameterFlag::Reference;
            return info;
        }
    };

    using TypeData = std::vector<uint64_t>; // TODO(arBmind): parser::Identifier

    static auto eval(const IdentifierLiterals& ids) -> TypeData { return {ids.v}; }

    static auto construct(const TypeData& typeData) -> api::Flags { return {typeData}; }
    static auto destruct(api::Flags& flags) { flags.~Flags(); }

    template<class Module>
    static void module(Module&) {
        // TODO(arBmind)
    }
};

} // namespace intrinsic
