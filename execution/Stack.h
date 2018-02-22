#pragma once
#include <cinttypes>
#include <memory>

namespace execution {

using Byte = uint8_t;

/**
 * initially allocated once never invalidates pointers!
 */
struct Stack {

    Stack(size_t total = 1024 * 1024);

    struct StackDeleter {
        Stack* stack;
        size_t size;

        auto operator()(void* p) -> void {
            if (p != nullptr) stack->free(size);
        }
    };
    using Ptr = std::unique_ptr<Byte, StackDeleter>;

    auto allocate(size_t) -> Ptr;
    void free(size_t);

private:
    std::unique_ptr<Byte[]> data;
    size_t total{};
    size_t used{};
};

} // namespace execution
