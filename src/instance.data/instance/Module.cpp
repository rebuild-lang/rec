#include "Module.h"

#include "Entry.h"

namespace instance {

namespace {

void fixTypes(Module* old, Module* cur) {
    for (auto& n : cur->locals) {
        n.visitSome([&](TypePtr& t) {
            if (t->module == old) t->module = cur;
        });
    }
}

} // namespace

Module::Module(This&& o) noexcept
    : name(std::move(o.name))
    , flags(std::move(o.flags))
    , locals(std::move(o.locals)) {
    fixTypes(&o, this);
}

auto Module::operator=(This&& o) & noexcept -> Module& {
    name = std::move(o.name);
    flags = std::move(o.flags);
    locals = std::move(o.locals);
    fixTypes(&o, this);
    return *this;
}

} // namespace instance
