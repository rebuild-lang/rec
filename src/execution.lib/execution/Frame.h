#pragma once
#include "Stack.h"

#include "instance/Views.h"

#include <map>

namespace execution {

using AddressByVariable = std::map<instance::VariableView, Byte*>;

struct LocalFrame {
    using This = LocalFrame;

public:
    LocalFrame() = default;
    ~LocalFrame() = default;

    // no copy
    LocalFrame(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move
    LocalFrame(This&&) = default;
    auto operator=(This &&) -> This& = default;

public:
    auto byVariable(instance::VariableView var) const& -> Byte* {
        auto it = m.find(var);
        if (it == m.end()) return nullptr;
        return it->second;
    }

    bool insert(instance::VariableView var, Byte* addr) & {
        auto [_, success] = m.try_emplace(var, addr);
        return success;
    }

private:
    AddressByVariable m;
};
// struct Frame {
//     using This = Frame;

//     const This* parent{};
//     LocalFrame locals;

//     explicit Frame(const This* parent)
//         : parent(parent) {}

//     Frame() = default;
//     ~Frame() = default;
//     // no copy
//     Frame(const This&) = delete;
//     auto operator=(const This&) -> This& = delete;
//     // move
//     Frame(This&&) = default;
//     auto operator=(This &&) -> This& = default;

// public:
//     auto byVariable(instance::VariableView var) const& -> Byte* {
//         auto addr = locals.byVariable(var);
//         if (addr) return addr;
//         if (parent) return parent->byVariable(var);
//         return nullptr;
//     }

//     bool insert(instance::VariableView var, Byte* addr) & { return locals.insert(var, addr); }
// };

} // namespace execution
