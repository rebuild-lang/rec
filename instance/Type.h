#pragma once
#include "meta/Flags.h"
#include "strings/String.h"

namespace instance {

using Name = strings::String;

enum class TypeFlag {
    CompileTime = 1 << 0,
    RunTime = 1 << 1,
};
using TypeFlags = meta::Flags<TypeFlag>;

struct Type {
    Name name{};
    uint64_t size{};
    TypeFlags flags{};
    // TODO
};
using TypeView = const Type*;

inline auto nameOf(const Type& type) -> const Name& { return type.name; }

} // namespace instance
