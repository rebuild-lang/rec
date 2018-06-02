#pragma once
#include "Stack.h"

#include "instance/Views.h"

#include <map>

namespace execution {

using AddressByTyped = std::map<instance::TypedView, Byte*>;

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
    auto operator[](instance::TypedView typed) const& -> Byte* {
        auto it = m.find(typed);
        if (it == m.end()) return nullptr;
        return it->second;
    }

    bool insert(instance::TypedView typed, Byte* addr) & {
        auto [_, success] = m.try_emplace(typed, addr);
        return success;
    }

private:
    AddressByTyped m;
};

struct Frame {
    using This = Frame;

    const This* parent{};
    LocalFrame locals;

    explicit Frame(const This* parent)
        : parent(parent) {}

    Frame() = default;
    ~Frame() = default;
    // no copy
    Frame(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move
    Frame(This&&) = default;
    auto operator=(This &&) -> This& = default;

public:
    auto operator[](instance::TypedView typed) const& -> Byte* {
        auto addr = locals[typed];
        if (addr) return addr;
        if (parent) return (*parent)[typed];
        return nullptr;
    }

    bool insert(instance::TypedView typed, Byte* addr) & { return locals.insert(typed, addr); }
};

} // namespace execution
