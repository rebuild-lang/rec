#include "Stack.h"

#include <cassert>

namespace execution {

Stack::Stack(size_t total)
    : data(new Byte[total])
    , total{total}
    , used{0} {}

auto Stack::allocate(size_t size) -> Ptr {
    if (size > 0 && used + size < total) {
        auto p = data.get() + used;
        used += size;
        return Ptr{p, StackDeleter{this, size}};
    }
    return {};
}

void Stack::free(size_t size) {
    assert(used >= size);
    used -= size;
}

} // namespace execution
