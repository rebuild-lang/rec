#pragma once
#include "instance/argument.h"
#include "instance/function.h"
#include "instance/variable.h"

#include "parser/block/block_token.h"

#include "scanner/text_range.h"

#include "tools/meta/variant.h"

#include <vector>

namespace parser::expression {

using text_range = scanner::text_range;
using view_t = scanner::view_t;
using name_t = strings::utf8_string;

class node_t;
using node_vec = std::vector<node_t>;

struct named_t;
using named_vec = std::vector<named_t>;

using string_literal_t = block::string_literal;
using number_literal_t = block::number_literal_t;
using operator_literal_t = block::operator_literal;
using identifier_literal_t = block::identifier_literal;
using block_literal_t = block::block_literal;

struct block_t {
    using this_t = block_t;
    node_vec nodes;

    bool operator==(const this_t &o) const { return nodes == o.nodes; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

struct argument_assignment {
    using this_t = argument_assignment;
    instance::argument_ptr argument{};
    node_vec values;

    bool operator==(const this_t &o) const { return argument == o.argument && values == o.values; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};
using argument_assignment_vec = std::vector<argument_assignment>;

struct invocation_t {
    using this_t = invocation_t;
    instance::function_ptr function{};
    argument_assignment_vec arguments;

    bool operator==(const this_t &o) const { return function == o.function && arguments == o.arguments; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

struct variable_reference_t {
    using this_t = variable_reference_t;
    instance::variable_ptr variable{};

    bool operator==(const this_t &o) const { return variable == o.variable; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

struct named_tuple_t {
    using this_t = named_tuple_t;
    named_vec tuple;

    bool operator==(const this_t &o) const { return tuple == o.tuple; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

using literal_variant =
    meta::variant<string_literal_t, number_literal_t, operator_literal_t, identifier_literal_t, block_literal_t>;

struct literal_t {
    using this_t = literal_t;
    literal_variant value;
    text_range range;

    bool operator==(const this_t &o) const { return value == o.value; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

using node_variant = meta::variant<block_t, invocation_t, variable_reference_t, named_tuple_t, literal_t>;

// note: this type is needed because we cannot forward a using definition
class node_t : public node_variant {
    using this_t = node_t;

public:
    // note: templated constructors are not forwarded with using
    template<
        class S,
        class... A,
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, this_t>> // ensure this does not capture a copy
        >
    node_t(S &&s, A &&... a)
        : node_variant(std::forward<S>(s), std::forward<A>(a)...) {}
};
using node_opt = meta::optional<node_t>;

struct named_t {
    using this_t = named_t;
    name_t name; // name might be empty!
    node_t node;

    named_t(name_t &&name, node_t &&node)
        : name(std::move(name))
        , node(std::move(node)) {}

    named_t(const this_t &) = default;
    named_t(this_t &&) = default;
    this_t &operator=(const this_t &) & = default;
    this_t &operator=(this_t &&) & = default;

    bool operator==(const this_t &o) const { return name == o.name && node == o.node; }
    bool operator!=(const this_t &o) const { return !(*this == o); }
};

auto operator<<(std::ostream &out, const node_t &) -> std::ostream &;
inline auto operator<<(std::ostream &out, const node_vec &ns) -> std::ostream & {
    auto size = ns.size();
    if (size > 1) out << "(";
    strings::join(out, ns, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream &out, const block_t &b) -> std::ostream & {
    if (b.nodes.empty()) {
        out << "{}\n";
    }
    else {
        out << "{\n  ";
        strings::join(out, b.nodes, "\n  ");
        out << "\n}\n";
    }
    return out;
}
inline auto operator<<(std::ostream &out, const argument_assignment &as) -> std::ostream & {
    return out << (as.argument ? as.argument->name : name_t("<?>")) << " = " << as.values;
}
inline auto operator<<(std::ostream &out, const invocation_t &inv) -> std::ostream & {
    out << (inv.function ? inv.function->name : name_t("<?>")) << "(";
    strings::join(out, inv.arguments, ", ");
    return out << ")";
}
inline auto operator<<(std::ostream &out, const variable_reference_t &vr) -> std::ostream & {
    return out << (vr.variable ? vr.variable->name : name_t("<?>"));
}
inline auto operator<<(std::ostream &out, const named_t &n) -> std::ostream & {
    if (n.name.is_empty()) {
        out << n.node;
    }
    else {
        out << n.name << " = " << n.node;
    }
    return out;
}
inline auto operator<<(std::ostream &out, const named_tuple_t &nt) -> std::ostream & {
    size_t size = nt.tuple.size();
    if (size > 1) out << "(";
    strings::join(out, nt.tuple, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream &out, const literal_t &lit) -> std::ostream & {
    out << "lit: ";
    lit.value.visit([&](const auto &a) { out << a; });
    return out;
}
inline auto operator<<(std::ostream &out, const node_t &n) -> std::ostream & {
    n.visit([&](const auto &a) { out << a; });
    return out;
}

} // namespace parser::expression
