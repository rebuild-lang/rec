#include "scanner/Tokenizer.h"

#include "filter/Filter.h"
#include "nesting/Parser.h"
#include "parser/Parser.h"

#include "instance/Scope.h"

#include "execution/Machine.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"

#include "nesting/Token.ostream.h"
#include "scanner/Token.ostream.h"

#include <iostream>

using ParserConfig = scanner::Config;
using InstanceScope = instance::Scope;
using InstanceNode = instance::Node;
using ExecutionContext = execution::Context;

using StringView = strings::View;
using Call = parser::Call;
using BlockLiteral = nesting::BlockLiteral;
using Block = parser::Block;
using OptNode = parser::OptNode;

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
            return parser::Parser::parse(blockLiteral, parserContext);
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

        return parser::Context{std::move(lookup), std::move(runCall), intrinsicType};
    }

public:
    Compiler(ParserConfig config, InstanceScope&& globals)
        : config(config)
        , globalScope(std::move(globals)) {
        compiler.parseBlock = [this](const nesting::BlockLiteral& block, InstanceScope* scope) {
            parser::Parser::parse(block, parserContext(*scope));
        };
    }

    auto compile(const text::File& file) -> Block {
        auto tokenize = [&](const auto& file) { return scanner::tokensFromFile(file, config); };
        auto filter = [&](const auto& file) { return filter::Filter::parse(tokenize(file)); };
        auto blockify = [&](const auto& file) { return nesting::Parser::parse(filter(file)); };
        auto parse = [&](const auto& file) {
            return parser::Parser::parse(blockify(file), parserContext(globalScope));
        };
        return parse(file);
        // TODO(arBmind): run phases
    }
};

int main() {
    auto config = scanner::Config{text::Column{8}};
    auto globals = instance::Scope{};
    globals.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    auto globalScope = instance::Scope{&globals};

    auto compiler = Compiler(config, std::move(globals));

    auto tokenize = [&](const auto& file) { return scanner::tokensFromFile(file, config); };
    auto filter = [&](const auto& file) { return filter::Filter::parse(tokenize(file)); };
    auto blockify = [&](const auto& file) { return nesting::Parser::parse(filter(file)); };

    auto file = text::File{strings::String{"TestFile"}, strings::String{R"(
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
