#pragma once
#include "expression_tree.h"

#include "expression_line_view.h"

#include "instance/scope_lookup.h"

namespace parser::expression {

using scope_t = instance::scope_t;
using function_t = instance::function_t;
using function_ptr = instance::function_ptr;
using block_literal_t = parser::block::block_literal;

struct parser {
    // TODO
    static auto parse(const block_literal_t &token_block, const scope_t &parent_scope) -> block_t { //
        auto scope = scope_t{&parent_scope};
        return parse_into(token_block, scope);
    }

    static auto parse_into(const block_literal_t &token_block, scope_t &scope) -> block_t { //
        auto block = block_t{};
        for (const auto &line : token_block.lines) {
            // TODO: split operators on the line
            auto it = line_view_t(&line);
            if (it) {
                auto expr = parse_tuple(it, scope);
                if (!expr.tuple.empty()) {
                    block.nodes.emplace_back(std::move(expr));
                }
                if (it) {
                    // TODO: report remaining tokens on line
                    // handling: ignore / maybe try to parse?
                }
            }
        }
        return block;
    }

private:
    static auto parse_tuple(line_view_t &it, scope_t &scope) -> named_tuple_t {
        auto tuple = named_tuple_t{};
        if (!it) return tuple;
        auto with_brackets = it.current().one_of<block::bracket_open>();
        if (with_brackets) ++it;
        parse_tuple_into(tuple, it, scope);
        if (with_brackets) {
            if (!it) {
                // error: missing closing bracket
            }
            else if (!it.current().one_of<block::bracket_close>()) {
                // error: missing closing bracket
            }
            else {
                ++it;
            }
        }
        return tuple;
    }

    static void parse_tuple_into(named_tuple_t &tuple, line_view_t &it, scope_t &scope) {
        while (it) {
            auto name = name_t{}; // parse_tuple_name(it);
            auto expr = parse_single(it, scope);
            if (expr) {
                tuple.tuple.emplace_back(std::move(name), std::move(expr).value());
            }
            else if (!name.is_empty()) {
                // TODO: named void?
            }

            if (!it) return;
            if (it.current().one_of<block::comma_separator>()) {
                ++it; // skip optional comma
                if (!it) return;
            }
            if (it.current().one_of<block::bracket_close>()) return;
        }
    }

    enum class parse_options {
        continue_single,
        finish_single,
    };

    static auto parse_single(line_view_t &it, scope_t &scope) -> node_opt {
        auto result = node_opt{};
        while (it) {
            auto opt = parse_step(result, it, scope);
            if (opt == parse_options::finish_single) break;
        }
        return result;
    }

    static auto parse_step(node_opt &result, line_view_t &it, scope_t &scope) -> parse_options {
        return it.current().data.visit(
            [](const block::comma_separator &) { return parse_options::finish_single; },
            [](const block::bracket_close &) { return parse_options::finish_single; },
            [&](const block::bracket_open &) {
                if (result) return parse_options::finish_single;
                auto tuple = parse_tuple(it, scope);
                result = node_opt{std::move(tuple)};
                return parse_options::continue_single;
            },
            [&](const block::identifier_literal &id) {
                const auto &instance = scope[it.current().range.text];
                if (!instance) {
                    if (result) return parse_options::finish_single;
                    result = node_opt{literal_t{id}};
                    ++it;
                    return parse_options::continue_single;
                }
                return parse_instance(result, *instance, it, scope);
            },
            [&](const block::string_literal &s) {
                if (result) return parse_options::finish_single;
                result = node_opt{literal_t{s}};
                ++it;
                return parse_options::continue_single;
            },
            [&](const block::number_literal_t &n) {
                if (result) return parse_options::finish_single;
                result = node_opt{literal_t{n}};
                ++it;
                return parse_options::continue_single;
            },
            [&](const block::block_literal &b) {
                if (result) return parse_options::finish_single;
                result = node_opt{literal_t{b}};
                ++it;
                return parse_options::continue_single;
            },
            [](const auto &) { return parse_options::finish_single; });
    }

    static auto parse_instance( //
        node_opt &result,
        const instance::variant_t &instance,
        line_view_t &it,
        scope_t &scope) -> parse_options {
        return instance.visit(
            [&](const instance::variable_t &var) {
                if (result) return parse_options::finish_single;
                result = node_opt{variable_reference_t{&var}};
                ++it;
                return parse_options::continue_single;
            },
            [&](const instance::argument_t &arg) {
                if (result) return parse_options::finish_single;
                // result = node_opt{argument_reference_t{&var}};
                ++it;
                return parse_options::continue_single;
            },
            [&](const instance::function_t &fun) {
                ++it;
                return parse_invocation(result, fun, it, scope);
            },
            [&](const instance::type_t &typ) {
                if (result) return parse_options::finish_single;
                // result = node_opt{type_reference_t{&typ}};
                ++it;
                return parse_options::continue_single;
            });
        // TODO: add modules
        // TODO: add overloads
    }

    static bool can_implicit_convert_to_type(node_ptr node, instance::type_ptr type) {
        // TODO:
        // I guess we need a scope here
        return true;
    }

    struct overload_set {
        struct overload {
            using this_t = overload;
            bool active = true;
            bool complete = false;
            function_ptr function;
            named_tuple_t right_args;

            overload() = default;
            overload(const function_t &function)
                : function(&function) {}

            overload(const this_t &) = default;
            overload(this_t &&) = default;
            auto operator=(const this_t &) & -> this_t & = default;
            auto operator=(this_t &&) & -> this_t & = default;

            void retire_left(const named_tuple_view_t &left) {
                auto o = 0u, t = 0u;
                auto la = function->left_arguments();
                for (const named_view_t &named : left.tuple) {
                    if (named.name.is_empty()) {
                        auto opt_arg = function->lookup_argument(named.name);
                        if (opt_arg) {
                            const instance::argument_t &arg = opt_arg.value();
                            if (arg.side == instance::argument_side::left //
                                && can_implicit_convert_to_type(named.node, arg.type)) {
                                t++;
                                continue;
                            }
                            // side does not match
                            // type does not match
                        }
                        // name not found
                    }
                    else if (o < la.size()) {
                        const auto &arg = la[o];
                        if (arg.side == instance::argument_side::left //
                            && can_implicit_convert_to_type(named.node, arg.type)) {
                            o++;
                            continue;
                        }
                        // side does not match
                        // type does not match
                    }
                    // index out of range
                    active = false;
                    return;
                }
                if (o + t == la.size()) return;
                // not right count
                active = false;
            }
        };
        using overload_vec = std::vector<overload>;

        overload_set(const function_t &fun) { vec.emplace_back(fun); }
        // TODO: multiple overloads

        void retire_left(const node_opt &left) {
            auto left_view = left ? left.value().holds<named_tuple_t>()
                                        ? named_tuple_view_t{left.value().get<named_tuple_t>()}
                                        : named_tuple_view_t{left.value()}
                                  : named_tuple_view_t{};
            for (auto &o : vec) o.retire_left(left_view);
            update();
        }

        auto active() const -> meta::vector_range<const overload> { return {vec.begin(), vec.begin() + active_count}; }

    private:
        void update() {
            auto it = std::stable_partition(
                vec.begin(), vec.begin() + active_count, [](const overload &o) { return o.active; });
            active_count = std::distance(vec.begin(), it);
        }

    private:
        overload_vec vec;
        size_t active_count{};
    };

    static auto parse_invocation( //
        node_opt &left,
        const function_t &fun,
        line_view_t &it,
        scope_t &scope) -> parse_options { //

        auto os = overload_set{fun};
        os.retire_left(left);
        if (!os.active().empty()) {
            //            auto backup = it;
            //            parse_arguments(os, it, scope);
            //            if (os.completed.size() == 1) {
            //                auto invocation = os.completed.front();
            //                // TODO: add compile time execution
            //                left = node_opt{invocation};
            //                return parse_options::continue_single;
            //            }
            //            it = backup;
        }
        //        if (left) return parse_options::finish_single;
        // left = node_opt{function_reference_t{fun}};
        return parse_options::continue_single;
    }
};

} // namespace parser::expression
