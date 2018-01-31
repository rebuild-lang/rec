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

using GenericFunc = void (*)();
#ifdef __clang__
    // clang refuses to work with GenericFunc
#    define asPtr(f) f
#else
    // VS2017 cannot use auto template arguments (needs GenericFunc)
#    define asPtr(f) (GenericFunc) f
#endif

} // namespace intrinsic
