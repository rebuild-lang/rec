#pragma once
#include "strings/String.h"

namespace instance {

using Name = strings::String;

struct Type {
    Name name;
    uint64_t size{};
    // TODO
};
using TypeView = const Type *;

inline auto nameOf(const Type &type) -> const Name & { return type.name; }

} // namespace instance
