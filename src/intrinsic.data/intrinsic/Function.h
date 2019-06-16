#pragma once
#include "meta/Flags.h"
#include "strings/View.h"

#include "Parameter.h"

#include <cinttypes>

namespace intrinsic {

using Name = strings::View;

enum class FunctionFlag : uint64_t {
    CompileTimeOnly = 1u << 0u,
    CompileTimeSideEffects = 1u << 1u, // side effects imply CompileTimeOnly for now!
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlags)

struct FunctionInfo {
    Name name{};
    FunctionFlags flags{};
};

template<class SigRet, class... SigArgs>
struct FunctionSignature {};

template<class Ret, class... Args>
auto makeSignature(Ret (*)(Args...)) -> FunctionSignature<Ret, Args...> {
    return {};
}

using FunctionInfoFunc = intrinsic::FunctionInfo (*)();

} // namespace intrinsic
