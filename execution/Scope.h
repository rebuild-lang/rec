#pragma once
#include "Stack.h"

#include "instance/Views.h"

#include <map>

namespace execution {

using AddressByTyped = std::map<instance::TypedView, Byte*>;

struct LocalScope {
    using This = LocalScope;

public:
    LocalScope() = default;
    ~LocalScope() = default;

    // no copy
    LocalScope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move
    LocalScope(This&&) = default;
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

struct Scope {
    using This = Scope;

    const This* parent{};
    LocalScope locals;

    explicit Scope(const This* parent)
        : parent(parent) {}

    Scope() = default;
    ~Scope() = default;
    // no copy
    Scope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move
    Scope(This&&) = default;
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
