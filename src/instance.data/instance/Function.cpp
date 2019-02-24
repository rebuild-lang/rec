#include "Function.h"

#include "Node.h"

namespace instance {

OptArgumentView Function::lookupArgument(NameView name) const {
    return argumentScope[name].map([](const auto node) -> OptArgumentView { return &node->template get<Argument>(); });
}

} // namespace instance
