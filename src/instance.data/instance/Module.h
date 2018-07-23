#pragma once
#include "LocalScope.h"

#include "meta/Flags.h"
#include "strings/String.h"
#include "strings/View.h"

namespace instance {

using Name = strings::CompareView;

enum class ModuleFlag {
    CompileTime = 1 << 0,
    RunTime = 1 << 1,
    Final = 1 << 2,
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
    auto operator=(const This&) & -> This& = delete;
    // movable
    Module(This&& o) noexcept;
    auto operator=(This&& o) & noexcept -> This&;
};
using ModuleView = const Module*;

inline auto nameOf(const Module& m) -> const Name& { return m.name; }

} // namespace instance
