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

using ParserConfig = scanner::Tokenizer::Config;
using InstanceScope = instance::Scope;
using InstanceNode = instance::Node;
using ExecutionContext = execution::Context;

using StringView = strings::View;
using Call = parser::expression::Call;
using BlockLiteral = parser::block::BlockLiteral;
using Block = parser::expression::Block;
using OptNode = parser::expression::OptNode;

class Compiler final {
    ParserConfig config;
    InstanceScope globalScope;

    execution::Compiler compiler;

    struct IntrinsicType {
        const InstanceScope& global;

        template<class T>
        auto operator()(meta::Type<T>) const -> instance::TypeView {
            return {};
        }
    };

    auto executionContext(InstanceScope& parserScope) {
        auto r = ExecutionContext{};
        r.compiler = &compiler;
        r.parserScope = &parserScope;
        auto parse = [&](const BlockLiteral& blockLiteral, auto& parserContext) {
            return parser::expression::Parser::parse(blockLiteral, parserContext);
        };
        auto declare = [&](InstanceNode&& node) { parserScope.emplace(std::move(node)); };
        (void)parse;
        (void)declare;
        return r;
    }

    auto parserContext(InstanceScope& scope) {
        auto lookup = [&](const StringView& id) { return scope[id]; };
        auto runCall = [&](const Call& call) -> OptNode {
            // TODOs:
            // * check arguments - have to be available
            // * create result arguments
            //

            // * create from caller context
            // * assign local scope
            execution::Machine::runCall(call, executionContext(scope));
            return {};
        };
        auto intrinsicType = IntrinsicType{globalScope};

        return parser::expression::Context{std::move(lookup), std::move(runCall), intrinsicType};
    }

public:
    Compiler(ParserConfig config, InstanceScope&& globals)
        : config(config)
        , globalScope(std::move(globals)) {
        compiler.parseBlock = [this](const parser::block::BlockLiteral& block, InstanceScope* scope) {
            parser::expression::Parser::parse(block, parserContext(*scope));
        };
    }

    auto compile(const scanner::File& file) -> Block {
        auto tokenize = [&](const auto& file) { return scanner::Tokenizer(config).scanFile(file); };
        auto filter = [&](const auto& file) { return parser::filter::Parser::parse(tokenize(file)); };
        auto blockify = [&](const auto& file) { return parser::block::Parser::parse(filter(file)); };
        auto parse = [&](const auto& file) {
            return parser::expression::Parser::parse(blockify(file), parserContext(globalScope));
        };
        return parse(file);
        // TODO: run phases
    }
};

int main() {
    auto config = scanner::Tokenizer::Config{scanner::Column{8}};
    auto globals = instance::Scope{};
    globals.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    auto globalScope = instance::Scope{&globals};

    auto compiler = Compiler(config, std::move(globals));

    auto tokenize = [&](const auto& file) { return scanner::Tokenizer(config).scanFile(file); };
    auto filter = [&](const auto& file) { return parser::filter::Parser::parse(tokenize(file)); };
    auto blockify = [&](const auto& file) { return parser::block::Parser::parse(filter(file)); };

    auto file = scanner::File{strings::String{"TestFile"}, strings::String{R"(
# Rebuild.say "Hello!"
Rebuild.Context.declareModule test:
    Rebuild.say "parsing inside!"
end
)"}};

    std::cout << "\nTokens:\n";
    for (auto t : tokenize(file)) std::cout << t << '\n';

    std::cout << "\nBlocks:\n";
    std::cout << blockify(file);

    std::cout << "\nParse:\n";
    compiler.compile(file);

    // std::cout << "Example = " << d.example() << '\n';
}
