#pragma once
#include "LocalScope.h"

#include "meta/Flags.h"
#include "strings/String.h"
#include "strings/View.h"

namespace instance {

using Name = strings::String;

enum class ModuleFlag {
    compile_time = 1 << 0, // marked as usable at compile time (applies to all new members)
    run_time = 1 << 1, // marked as usable at run time (applies to all new members)
    final = 1 << 2, // marked as final (no new members allowed, applies to all submodules)
};
using ModuleFlags = meta::Flags<ModuleFlag>;

struct Module {
    using This = Module;

    Name name{};
    ModuleFlags flags{};
    LocalScope locals{};

    Module() = default;
    ~Module() = default;
    // no copy
    Module(const This&) = delete;
    auto operator=(const This&) & -> Module& = delete;
    // movable
    Module(This&& o) noexcept;
    auto operator=(This&& o) & noexcept -> Module&;
};
using ModulePtr = std::shared_ptr<Module>;
using ModuleView = const Module*;

} // namespace instance
