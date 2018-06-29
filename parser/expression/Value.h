#pragma once
#include "TypeTree.h"

namespace parser::expression {

struct Value {
    template<class T>
    Value(T&& v, TypeExpression&& t)
        : m(std::make_shared<Implementation<T>>(std::move(t), std::move(v))) {}

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

private:
    struct Interface {
        TypeExpression t{};

        explicit Interface(TypeExpression&& t)
            : t(std::move(t)) {}

        Interface() = default;
        Interface(Interface&&) noexcept = default;
        Interface& operator=(Interface&&) noexcept = default;
        Interface(const Interface&) = default;
        Interface& operator=(const Interface&) = default;

        virtual ~Interface() = default;
        virtual auto data() const -> const void* = 0;
        virtual auto data() -> void* = 0;
        virtual auto operator==(const Interface&) const -> bool = 0;
    };

    template<class T>
    struct Implementation : Interface {
        T v;

        Implementation(TypeExpression&& t, T&& v)
            : Interface(std::move(t))
            , v(std::move(v)) {}

        Implementation() = default;
        Implementation(Implementation&&) noexcept = default;
        Implementation& operator=(Implementation&&) noexcept = default;
        Implementation(const Implementation&) = default;
        Implementation& operator=(const Implementation&) = default;

        ~Implementation() override = default;
        auto data() const -> const void* override { return &v; }
        auto data() -> void* override { return &v; }
        auto operator==(const Interface& o) const -> bool override {
            return v == static_cast<const Implementation&>(o).v;
        }
    };

    std::shared_ptr<Interface> m;
};

} // namespace parser::expression
