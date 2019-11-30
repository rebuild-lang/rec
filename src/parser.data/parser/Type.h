#pragma once
#include "instance/Views.h"

#include "meta/Optional.h"

#if !defined(VALUE_DEBUG_DATA)
#    if defined(_DEBUG)
#        define VALUE_DEBUG_DATA
#    endif
#endif

#ifdef VALUE_DEBUG_DATA
#    include <ostream>
#endif

namespace parser {

using instance::ModuleView;

using ConstructFunc = void(void* dest);
using DestructFunc = void(void* dest);
using CloneFunc = void(void* dest, const void* source);
using EqualFunc = bool(const void*, const void*);
#ifdef VALUE_DEBUG_DATA
using DebugDataFunc = auto(std::ostream& out, const void*) -> std::ostream&;
#endif

enum class TypeParser {
    Expression,
    SingleToken,
    IdTypeValue,
};

struct Type {
    ModuleView module{};
    uint64_t size{};
    uint64_t alignment{};
    ConstructFunc* constructFunc{};
    DestructFunc* destructFunc{};
    CloneFunc* cloneFunc{};
    EqualFunc* equalFunc{};
    TypeParser typeParser{};
#ifdef VALUE_DEBUG_DATA
    DebugDataFunc* debugDataFunc{};
#endif
};
using TypeView = const Type*;
using OptTypeView = meta::Optional<TypeView>;

} // namespace parser
