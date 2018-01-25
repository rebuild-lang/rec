#pragma once
#include "meta/Flags.h"
#include "strings/View.h"

#include "Argument.h"

#include <cinttypes>

namespace intrinsic {

using Name = strings::View;

enum class FunctionFlag : uint64_t {
    CompileTimeOnly = 1 << 0,
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlags)

struct FunctionInfo {
    Name name{};
    FunctionFlags flags{};
};

} // namespace intrinsic
