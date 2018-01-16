#pragma once
#include "tools/meta/Flags.h"

#include "Argument.h"

#include <cinttypes>

namespace intrinsic {

enum class FunctionFlag : uint64_t {
    CompileTimeOnly = 1 << 0,
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlags)

struct FunctionInfo {
    const char* name{};
    FunctionFlags flags{};
};

} // namespace intrinsic
