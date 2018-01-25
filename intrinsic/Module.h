#pragma once
#include "strings/View.h"

namespace intrinsic {

using Name = strings::View;

struct ModuleInfo {
    Name name{};
};

/* a module should look like this:
 *
 * struct MyModule {
 *   constexpr static auto info() -> ModuleInfo;
 *
 *   template<class Module>
 *   constexpr static auto module(Module& mod) {
 *      mod.template type<Type>();
 *      mod.template function<[]() -> FunctionInfo {}>(&func);
 *      mod.template module<OtherModule>();
 *   }
 * };
 *
 * see ModuleOutput as example iterator
 */

} // namespace intrinsic
