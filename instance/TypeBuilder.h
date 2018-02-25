#pragma once
#include "Module.h"
#include "Type.h"

namespace instance {

namespace details {

struct TypeBuilder {
    Module mod_;
    Type type_;

    template<size_t N>
    TypeBuilder(const char (&name)[N]) {
        mod_.name = Name{name};
        type_.name = Name{"type"};
    }

    auto size(size_t size) && -> TypeBuilder {
        type_.size = size;
        return std::move(*this);
    }

    auto build() && -> Module {
        type_.module = &mod_;
        mod_.locals.emplace(std::move(type_));
        return std::move(mod_);
    }
};

} // namespace details

template<size_t N>
auto typeMod(const char (&name)[N]) -> details::TypeBuilder {
    return {name};
}

} // namespace instance
