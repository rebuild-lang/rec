#include "expression_parser.h"
#include "expression_tree_builder.h"

#include "parser/block/block_token_builder.h"

#include "instance/function_builder.h"
#include "instance/scope_builder.h"

#include "gtest/gtest.h"

#include <memory>

using namespace parser;
using namespace parser::expression;

using scope_t = instance::scope_t;
using block_line = block::line;

struct expression_parser_data {
    const char *name;
    std::shared_ptr<scope_t> scope;
    block_line input;
    block_t expected;

    expression_parser_data(const char *name)
        : name{name}
        , scope{std::make_shared<scope_t>()} {}
    expression_parser_data(const expression_parser_data &) = default;
    expression_parser_data &operator=(const expression_parser_data &) = default;
    expression_parser_data(expression_parser_data &&) = default;
    expression_parser_data &operator=(expression_parser_data &&) = default;

    template<class... Instance>
    auto ctx(Instance &&... instance) && -> expression_parser_data {
        instance::build_scope(*scope, std::forward<Instance>(instance)...);
        return std::move(*this);
    }

    template<class... Token>
    auto in(Token &&... token) && -> expression_parser_data {
        input = block_line{block::build_token(std::forward<Token>(token))...};
        return std::move(*this);
    }

    template<class... Expr>
    auto out(Expr &&... expr) && -> expression_parser_data {
        expected.nodes.reserve(expected.nodes.size() + sizeof...(Expr));
        auto x = {
            (expected.nodes.emplace_back(expression::build_expr(*scope, std::forward<Expr>(expr))), 0)... //
        };
        (void)x;
        return std::move(*this);
    }
};

static auto operator<<(std::ostream &out, const expression_parser_data &epd) -> std::ostream & {
    out << "name: " << epd.name << "\n";
    out << "input:\n";
    out << epd.input << '\n';
    out << "expected:\n";
    // TODO
    return out;
}

class expression_parser : public testing::TestWithParam<expression_parser_data> {};

TEST_P(expression_parser, calls) {
    const expression_parser_data &data = GetParam();
    const auto input = block::block_literal{{data.input}};
    const auto &scope = data.scope;
    const auto &expected = data.expected;

    auto blk = expression::parser::parse(input, *scope);

    ASSERT_EQ(blk, expected);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    expression_parser,
    ::testing::Values( //
        expression_parser_data("Example") //
            .ctx( //
                instance::fun("print").args(instance::arg("v").right()) // .type("rebuild.number_literal")
                )
            .in(block::id("print"), block::num("1"))
            .out(expression::invoke("print").right(arg("v", literal_variant(block::num("1"))))) //
        ));
