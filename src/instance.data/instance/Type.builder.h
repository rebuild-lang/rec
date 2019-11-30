#pragma once
#include "Module.h"
#include "Type.h"

#include "parser/Type.builder.h"

namespace instance {

namespace details {

struct TypeModuleBuilder {
    using This = TypeModuleBuilder;
    Module mod_;
    Type type_;

    template<size_t N>
    explicit TypeModuleBuilder(const char (&name)[N]) {
        mod_.name = Name{name};
    }

    auto size(size_t size) && -> This {
        type_.size = size;
        return std::move(*this);
    }
    auto align(size_t align) && -> This {
        type_.alignment = align;
        return std::move(*this);
    }
    auto construct(parser::ConstructFunc* f) && -> This {
        type_.constructFunc = f;
        return std::move(*this);
    }
    auto destruct(parser::DestructFunc* f) && -> This {
        type_.destructFunc = f;
        return std::move(*this);
    }

    auto clone(parser::CloneFunc* f) && -> This {
        type_.cloneFunc = f;
        return std::move(*this);
    }
    auto equal(parser::EqualFunc* f) && -> This {
        type_.equalFunc = f;
        return std::move(*this);
    }
#ifdef VALUE_DEBUG_DATA
    auto debugData(parser::DebugDataFunc* f) && -> This {
        type_.debugDataFunc = f;
        return std::move(*this);
    }
#endif
    auto parser(parser::TypeParser parser) && -> This {
        type_.typeParser = parser;
        return std::move(*this);
    }

    auto build() && -> Module {
        auto ptr = &mod_;
        type_.module = ptr;
        mod_.locals.emplace(std::move(type_));
        return std::move(mod_); // note: move constructor fixes module ptr in type!
    }
};

} // namespace details

template<size_t N>
auto typeMod(const char (&name)[N]) {
    return details::TypeModuleBuilder{name};
}

template<class T, size_t N>
auto typeModT(const char (&name)[N]) {
    return details::TypeModuleBuilder{name}
        .size(sizeof(T))
        .align(alignof(T))
        .construct([](void* dest) { new (dest) T(); })
        .destruct([](void* dest) { std::launder(reinterpret_cast<T*>(dest))->~T(); })
        .clone([](void* dest, const void* source) { new (dest) T(*std::launder(reinterpret_cast<const T*>(source))); })
        .equal([](const void* a, const void* b) -> bool {
            return *std::launder(reinterpret_cast<const T*>(a)) == *std::launder(reinterpret_cast<const T*>(b));
        })
#ifdef VALUE_DEBUG_DATA
        .debugData([](std::ostream& out, const void* dest) -> std::ostream& {
            return out << *std::launder(reinterpret_cast<const T*>(dest));
        })
#endif
        ;
}

} // namespace instance
