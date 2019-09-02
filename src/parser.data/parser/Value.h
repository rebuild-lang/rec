#pragma once
#include "TypeTree.h"

#if !defined(VALUE_DEBUG_DATA)
#    if defined(GTEST_LANG_CXX11)
#        define VALUE_DEBUG_DATA
#    endif
#endif

namespace parser {

struct Value {
    template<class T>
    Value(T&& v, TypeExpression&& t)
        : m(std::make_shared<Implementation<std::remove_reference_t<T>>>(std::move(t), std::move(v))) {}

    template<class T>
    static auto uninitialized(const TypeExpression& t) -> Value {
        return Value{std::make_shared<Uninitialized<std::remove_reference_t<T>>>(t)};
    }

    Value() = default;
    Value(Value&&) noexcept = default;
    Value& operator=(Value&&) noexcept = default;
    Value(const Value&) = default;
    Value& operator=(const Value&) = default;
    ~Value() = default;

    auto type() const& -> const TypeExpression& { return m->t; }
    auto data() const& -> const void* { return m->data(); }
    auto data() & -> void* { return m->data(); }

    using This = Value;
    bool operator==(const This& o) const { return m == o.m || *m == *o.m; }
    bool operator!=(const This& o) const { return !(*this == o); }
#ifdef VALUE_DEBUG_DATA
    auto debugData(std::ostream& out) const -> std::ostream& { return m->debugData(out); }
#endif

private:
    struct Virtual {
        using Self = void*;

        using DataConst = auto(const void*) -> const void*;
        DataConst* dataConst;

        using Data = auto(void*) -> void*;
        Data* data;

        using Equal = bool(const void*, const void*);
        Equal* equal;

#ifdef VALUE_DEBUG_DATA
        using DebugData = auto(const void*, std::ostream&) -> std::ostream&;
        DebugData* debugData;
#endif
    };

    struct Interface {
        const Virtual* vptr{};
        TypeExpression t{};

        explicit Interface(const Virtual* vptr, TypeExpression&& t)
            : vptr(vptr)
            , t(std::move(t)) {}

        Interface() = default;
        Interface(Interface&&) noexcept = default;
        Interface& operator=(Interface&&) noexcept = default;
        Interface(const Interface&) = default;
        Interface& operator=(const Interface&) = default;

        auto data() const -> const void* { return vptr->dataConst(this); }
        auto data() -> void* { return vptr->data(this); }
        auto operator==(const Interface& o) const -> bool { return vptr->equal(this, &o); }
#ifdef VALUE_DEBUG_DATA
        auto debugData(std::ostream& out) const -> std::ostream& { return vptr->debugData(this, out); }
#endif
    };

    template<class T>
    static auto implementationVirtual() -> const Virtual* {
        static auto d = Virtual{
            [](const void* self) -> const void* { return reinterpret_cast<const Implementation<T>*>(self)->data(); },
            [](void* self) -> void* { return reinterpret_cast<Implementation<T>*>(self)->data(); },
            [](const void* l, const void* r) -> bool {
                return *reinterpret_cast<const Implementation<T>*>(l) == *reinterpret_cast<const Implementation<T>*>(r);
            }
#ifdef VALUE_DEBUG_DATA
            ,
            [](const void* self, std::ostream& out) -> std::ostream& {
                return reinterpret_cast<const Implementation<T>*>(self)->debugData(out);
            }
#endif
        };
        return &d;
    }

    template<class T>
    struct Uninitialized : Interface {
        uint8_t data[sizeof(T)];

        Uninitialized(const TypeExpression& t)
            : Interface(implementationVirtual<T>(), TypeExpression{t}) {}
    };

    template<class T>
    struct Implementation : Interface {
        T v;

        Implementation(TypeExpression&& t, T&& v)
            : Interface(implementationVirtual<T>(), std::move(t))
            , v(std::move(v)) {}

        Implementation() = default;
        Implementation(Implementation&&) noexcept = default;
        Implementation& operator=(Implementation&&) noexcept = default;
        Implementation(const Implementation&) = default;
        Implementation& operator=(const Implementation&) = default;

        ~Implementation() = default;
        auto data() const -> const void* { return &v; }
        auto data() -> void* { return &v; }
        auto operator==(const Interface& o) const -> bool { return v == static_cast<const Implementation&>(o).v; }
#ifdef VALUE_DEBUG_DATA
        auto debugData(std::ostream& out) const -> std::ostream& { return out << v; }
#endif
    };

    std::shared_ptr<Interface> m;

    Value(std::shared_ptr<Interface>&& ptr)
        : m(std::move(ptr)) {}
};

} // namespace parser
