#pragma once
#include "Type.h"

#include "meta/TypeTraits.h"

namespace parser {

struct Value {
    using This = Value;

    Value() = default;
    explicit Value(TypeView type)
        : m_type(type)
        , m_storage(m_type ? new uint8_t[type->size] : nullptr) {
        if (m_type) m_type->constructFunc(data());
    }
    ~Value() { destruct(); }

    Value(const This& o)
        : m_type(o.m_type)
        , m_storage(m_type ? new uint8_t[o.m_type->size] : nullptr) {
        if (m_type) m_type->cloneFunc(data(), o.data());
    }
    auto operator=(const This& o) -> This& {
        if (m_type != nullptr) {
            if (m_storage) m_type->destructFunc(data());
            if (o.m_type == nullptr) {
                m_storage.reset(nullptr);
                m_type = nullptr;
            }
            else {
                if (m_type != o.m_type) {
                    m_type = o.m_type;
                    if (m_type->size != o.m_type->size) m_storage.reset(new uint8_t[o.m_type->size]);
                }
                m_type->cloneFunc(data(), o.data());
            }
        }
        else if (m_type != nullptr) {
            m_type = o.m_type;
            m_storage.reset(new uint8_t[o.m_type->size]);
            m_type->cloneFunc(data(), o.data());
        }
        return *this;
    }

    Value(This&& o) noexcept
        : m_type(o.m_type)
        , m_storage(o.m_storage.release()) {}

    auto operator=(This&& o) noexcept -> This& {
        destruct();
        m_type = o.m_type;
        m_storage = std::move(o.m_storage);
        return *this;
    }

    bool operator==(const This& o) const {
        return m_type == o.m_type && (m_type == nullptr || m_type->equalFunc(data(), o.data()));
    }
    bool operator!=(const This& o) const { return !(*this == o); }

    auto type() const& -> TypeView { return m_type; }

    auto data() const& -> const void* { return m_storage.get(); }
    auto data() & -> void* { return m_storage.get(); }

    template<class T>
    auto get() const& -> const T& {
        return *std::launder(reinterpret_cast<const T*>(data()));
    }
    template<class T>
    auto set() & -> T& {
        return *std::launder(reinterpret_cast<T*>(data()));
    }

private:
    TypeView m_type{};
    std::unique_ptr<uint8_t[]> m_storage{};

    void destruct() noexcept {
        if (m_type && m_storage) m_type->destructFunc(data());
    }
};
static_assert(meta::has_move_assignment<Value>);

} // namespace parser
