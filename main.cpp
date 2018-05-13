#include "parser/block/Parser.h"
#include "parser/expression/Parser.h"
#include "parser/filter/Parser.h"
#include "scanner/Tokenizer.h"

#include "instance/Scope.h"

#include "execution/Machine.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"

#include "parser/block/TokenOutput.h"
#include "scanner/TokenOutput.h"

#include <iostream>

int main() {
    auto config = scanner::Tokenizer::Config{scanner::Column{8}};
    auto intrinsics = instance::Scope{};
    intrinsics.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    auto scope = instance::Scope{&intrinsics};
    auto lookup = [&](const strings::View& id) { return scope[id]; };

    auto compiler = execution::Compiler{};
    auto executionContext = [&] {
        auto r = execution::Context{};
        r.compiler = &compiler;
        return r;
    }();

    auto runCall = [&](const parser::expression::Call& call) -> parser::expression::OptNode {
        // TODOs:
        // * check arguments - have to be available
        // * create result arguments
        //

        // * create from caller context
        // * assign local scope
        execution::Machine::runCall(call, executionContext);
        return {};
    };

    auto parserContext = parser::expression::makeContext(std::move(lookup), std::move(runCall));

    auto tokenize = [&](const auto& file) { return scanner::Tokenizer(config).scanFile(file); };
    auto filter = [&](const auto& file) { return parser::filter::Parser::parse(tokenize(file)); };
    auto blockify = [&](const auto& file) { return parser::block::Parser::parse(filter(file)); };
    auto parse = [&](const auto& file) { return parser::expression::Parser::parse(blockify(file), parserContext); };

    auto file = scanner::File{strings::String{"TestFile"}, strings::String{R"(
Rebuild.say "Hello!"
)"}};

    std::cout << "\nTokens:\n";
    for (auto t : tokenize(file)) std::cout << t << '\n';

    std::cout << "\nBlocks:\n";
    std::cout << blockify(file);

    std::cout << "\nParse:\n";
    parse(file);

    // std::cout << "Example = " << d.example() << '\n';
}
