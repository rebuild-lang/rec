#pragma once
#include "Module.h"
#include "Type.h"

namespace instance {

namespace details {

struct TypeBuilder {
    Module mod_;
    Type type_;

    template<size_t N>
    explicit TypeBuilder(const char (&name)[N]) {
        mod_.name = Name{name};
    }

    auto size(size_t size) && -> TypeBuilder {
        type_.size = size;
        return std::move(*this);
    }

    auto clone(CloneFunc* f) && -> TypeBuilder {
        type_.clone = f;
        return std::move(*this);
    }

    auto parser(Parser parser) && -> TypeBuilder {
        type_.parser = parser;
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
auto typeMod(const char (&name)[N]) {
    return details::TypeBuilder{name};
}

template<class T, size_t N>
auto typeModT(const char (&name)[N]) {
    return details::TypeBuilder{name}.size(sizeof(T)).clone(
        [](uint8_t* dest, const uint8_t* source) { new (dest) T(*reinterpret_cast<const T*>(source)); });
}

} // namespace instance
