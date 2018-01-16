#pragma once
#include "tools/meta/Flags.h"

#include <cinttypes>

namespace intrinsic {

enum class TypeFlag : uint64_t {
    CompileTime = 1 << 0,
    RunTime = 1 << 1,
    Construct = 1 << 2,
    Instance = 1 << 3,
};
using TypeFlags = meta::Flags<TypeFlag>;
META_FLAGS_OP(TypeFlags)

struct TypeInfo {
    const char* name{};
    uint64_t size{};
    TypeFlags flags{};
};

/* specialize for usage
 *
 * template<>
 * struct TypeOf<int> {
 *   static constexpr auto info = []() -> TypeInfo { … }();
 *   static void module(Module&) { … }
 * };
 */
template<class>
struct TypeOf;

} // namespace intrinsic
