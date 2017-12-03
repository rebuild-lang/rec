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
                    if (expr.tuple.size() == 1 && expr.tuple.front().name.is_empty()) {
                        // no reason to keep the tuple around, unwrap it
                        block.nodes.emplace_back(std::move(expr).tuple.front().node);
                    }
                    else {
                        block.nodes.emplace_back(std::move(expr));
                    }
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
            auto opt = parse_single_named(it, scope);
            if (opt) {
                tuple.tuple.push_back(std::move(opt).value());
            }
            auto r = parse_optional_comma(it);
            if (r == parse_options::finish_single) break;
        }
    }

    enum class parse_options {
        continue_single,
        finish_single,
    };

    static parse_options parse_optional_comma(line_view_t &it) {
        if (!it) return parse_options::finish_single;
        if (it.current().one_of<block::comma_separator>()) {
            ++it; // skip optional comma
            if (!it) return parse_options::finish_single;
        }
        if (it.current().one_of<block::bracket_close>()) return parse_options::finish_single;
        return parse_options::continue_single;
    }

    static auto parse_single_named(line_view_t &it, scope_t &scope) -> named_opt {
        auto name = name_t{}; // parse_tuple_name(it);
        auto expr = parse_single(it, scope);
        if (expr) {
            return named_t{std::move(name), std::move(expr).value()};
        }
        else if (!name.is_empty()) {
            // TODO: named void?
        }
        return {};
    }

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
        const instance::node_t &instance,
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
            },
            [&](const instance::module_t &mod) {
                if (result) return parse_options::finish_single;
                // result = node_opt{module_reference_t{&typ}};
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
            line_view_t it;
            argument_assignment_vec right_args;
            size_t next_arg{};

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
                    if (!named.name.is_empty()) {
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

            auto arg() const -> const instance::argument_t & { return function->right_arguments()[next_arg]; }
        };
        using overload_vec = std::vector<overload>;

        overload_set(const function_t &fun) {
            vec.emplace_back(fun);
            active_count = 1;
        }
        // TODO: allow multiple overloads

        void retire_left(const node_opt &left) {
            auto left_view = left ? left.value().holds<named_tuple_t>()
                                        ? named_tuple_view_t{left.value().get<named_tuple_t>()}
                                        : named_tuple_view_t{left.value()}
                                  : named_tuple_view_t{};
            for (auto &o : vec) o.retire_left(left_view);
            update();
        }

        void setup_it(line_view_t &it) {
            for (auto &o : vec) o.it = it;
        }

        auto active() const & -> meta::vector_range<const overload> {
            return {vec.begin(), vec.begin() + active_count};
        }
        auto active() & -> meta::vector_range<overload> { return {vec.begin(), vec.begin() + active_count}; }

        void update() {
            auto it = std::stable_partition(
                vec.begin(), vec.begin() + active_count, [](const overload &o) { return o.active; });
            active_count = std::distance(vec.begin(), it);
        }
        auto finish() & -> meta::vector_range<overload> {
            auto it = std::stable_partition(vec.begin(), vec.end(), [](const overload &o) { return o.complete; });
            return {vec.begin(), it};
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
        if (!os.active().empty() && it) {
            parse_arguments(os, it, scope);
            auto completed = os.finish();
            if (completed.size() == 1) {
                auto &o = completed.front();
                it = o.it;
                // TODO: add compile time execution
                auto inv = invocation_t{};
                inv.function = o.function;
                // TODO: assign left arguments
                inv.arguments = o.right_args;
                left = node_opt{{inv}};
                return parse_options::continue_single;
            }
        }
        if (left) return parse_options::finish_single;
        // left = node_opt{function_reference_t{fun}};
        return parse_options::continue_single;
    }

    static void parse_arguments(overload_set &os, line_view_t &it, scope_t &scope) {
        auto with_brackets = it.current().one_of<block::bracket_open>();
        if (with_brackets) ++it;
        parse_arguments_without(os, it, scope);
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
    }

    static void parse_arguments_without(overload_set &os, line_view_t &it, scope_t &scope) {
        os.setup_it(it);
        while (!os.active().empty()) {
            line_view_t base_it = it;
            // TODO: optimize for no custom parser case!
            for (auto &o : os.active()) {
                const auto &a = o.arg();
                // auto p = nullptr; // parser_for_type(a.type);
                auto opt_named = parse_single_named(o.it, scope); // : p.parse(o.it, scope);
                if (opt_named) {
                    named_t &named = opt_named.value();
                    if (!named.name.is_empty()) {
                        auto opt_arg = o.function->lookup_argument(named.name);
                        if (opt_arg) {
                            const instance::argument_t &arg = opt_arg.value();
                            if (can_implicit_convert_to_type(&named.node, a.type)) {
                                auto as = argument_assignment{};
                                as.argument = &arg;
                                as.values = {std::move(opt_named).value().node};
                                o.right_args.push_back(std::move(as));
                            }
                            else {
                                // type does not match
                            }
                        }
                        else {
                            // name not found
                        }
                    }
                    else {
                        if (can_implicit_convert_to_type(&named.node, a.type)) {
                            auto as = argument_assignment{};
                            as.argument = &a;
                            as.values = {std::move(opt_named).value().node};
                            o.right_args.push_back(std::move(as));
                            o.next_arg++;
                        }
                        else {
                            // type does not match
                        }
                    }
                }
                else {
                    // no value
                }

                if (o.next_arg == o.function->right_arguments().size()) {
                    o.complete = true;
                    o.active = false;
                }
                else {
                    auto r = parse_optional_comma(o.it);
                    if (r == parse_options::finish_single) {
                        o.active = false;
                    }
                }
            }
            os.update();
        }
    }
};

} // namespace parser::expression
