#include "scanner/tokenize.h"
#include "strings/utf8Decode.h"
#include "text/decodePosition.h"

#include "filter/filterTokens.h"
#include "nesting/nestTokens.h"
#include "parser/Parser.h"

#include "instance/Scope.h"

#include "execution/Machine.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"
#include "intrinsic/ResolveType.h"

#include "diagnostic/Diagnostic.ostream.h"
#include "nesting/Token.ostream.h"
#include "scanner/Token.ostream.h"

#include <iostream>

using ParserConfig = text::Config;
using InstanceScope = instance::Scope;
using InstanceNode = instance::Node;
using ExecutionContext = execution::Context;

using StringView = strings::View;
using diagnostic::Diagnostic;
using diagnostic::Diagnostics;
using nesting::BlockLiteral;
using parser::Block;
using parser::Call;
using parser::OptNode;

class Compiler final {
    ParserConfig config;
    InstanceScope globals;
    InstanceScope globalScope;
    Diagnostics diagnostics;

    execution::Compiler compiler;

    struct IntrinsicType {
        const InstanceScope* globals;

        template<class T>
        auto operator()(meta::Type<T>) const -> instance::TypeView {
            return intrinsic::ResolveType<T>::template moduleInstance<intrinsic::Rebuild>(globals);
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

    static auto assignResultStorage(Call& call) {
        const auto* f = call.function;
        for (auto* a : f->arguments) {
            if (a->side == instance::ArgumentSide::result //
                && a->typed.type.holds<parser::Pointer>()) {

                parser::TypeExpression* ptrTarget = a->typed.type.get<parser::Pointer>().target.get();
                ptrTarget->visit(
                    [&](const parser::Pointer& p) {
                        call.arguments.push_back({a, {parser::Value{nullptr, parser::TypeExpression{*ptrTarget}}}});
                    },
                    [&](const parser::TypeInstance& i) {
                        call.arguments.push_back({a, {i.concrete->makeUninitialized({*ptrTarget})}});
                    },
                    [](const auto) {});
            }
        }
    }

    static auto getResultValue(Call& call) -> meta::Optional<parser::Value> {
        auto result = meta::Optional<parser::Value>{};
        for (auto& a : call.arguments) {
            if (a.argument->side == instance::ArgumentSide::result //
                && a.argument->typed.type.holds<parser::Pointer>()) {

                if (result || a.values.size() != 1 || !a.values[0].holds<parser::Value>()) return {};

                result = a.values[0].get<parser::Value>();
            }
        }
        return result;
    }

    static auto extractResults(Call& call, const InstanceScope& globals) -> OptNode {
        auto optResult = getResultValue(call);
        if (optResult) {
            auto& resultType = optResult.value().type();
            if (resultType.holds<parser::TypeInstance>() &&
                resultType.get<parser::TypeInstance>().concrete ==
                    IntrinsicType{&globals}(meta::Type<parser::VariableInit>{})) {

                return parser::Node{*reinterpret_cast<parser::VariableInit*>(optResult.value().data())};
            }
        }
        // TODO(arBmind): create TypedTuple from results
        return {};
    }

    auto parserContext(InstanceScope& scope) {
        auto lookup = [&](const StringView& id) { return scope[id]; };
        auto runCall = [&](const Call& call) -> OptNode {
            // TODO(arBmind):
            // * check arguments - have to be available
            auto callCopy = call;
            assignResultStorage(callCopy);

            execution::Machine::runCall(callCopy, executionContext(scope));

            return extractResults(callCopy, globals);
        };
        auto reportDiagnostic = [this](Diagnostic diagnostic) {
            // TODO(arBmind): somehow add fileName
            diagnostics.emplace_back(std::move(diagnostic));
        };
        return parser::Context{
            std::move(lookup), std::move(runCall), IntrinsicType{&globals}, std::move(reportDiagnostic)};
    }

    void printDiagnostics() {
        std::cout << diagnostics.size() << " diagnostics created\n";
        for (auto& d : diagnostics) std::cout << d;
    }

public:
    Compiler(ParserConfig config, InstanceScope&& globals)
        : config(config)
        , globals(std::move(globals))
        , globalScope(&this->globals) {
        compiler.parseBlock = [this](const nesting::BlockLiteral& block, InstanceScope* scope) -> parser::Block {
            return parser::Parser::parse(block, parserContext(*scope));
        };
    }
    Compiler(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    void compile(const text::File& file) {
        auto decode = [&](const auto& file) { return strings::utf8Decode(file.content); };
        auto positions = [&](const auto& file) { return text::decodePosition(decode(file), config); };
        auto tokenize = [&](const auto& file) { return scanner::tokenize(positions(file)); };
        auto filter = [&](const auto& file) { return filter::filterTokens(tokenize(file)); };
        auto blockify = [&](const auto& file) { return nesting::nestTokens(filter(file)); };
        auto parse = [&](const auto& file) {
            return parser::Parser::parse(blockify(file), parserContext(globalScope));
        };
        auto block = parse(file);
        if (!diagnostics.empty())
            printDiagnostics();
        else
            execution::Machine::runBlock(block, executionContext(globals));
    }
};

int main() {
    auto config = ParserConfig{text::Column{8}};
    auto globals = instance::Scope{};
    globals.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    // auto globalScope = instance::Scope{&globals};

    auto compiler = Compiler{config, std::move(globals)};

    // auto decode = [&](const auto& file) { return strings::utf8Decode(file.content); };
    // auto positions = [&](const auto& file) { return text::decodePosition(decode(file), config); };
    // auto tokenize = [&](const auto& file) { return scanner::tokenize(positions(file)); };
    // auto filter = [&](const auto& file) { return filter::filterTokens(tokenize(file)); };
    // auto blockify = [&](const auto& file) { return nesting::nestTokens(filter(file)); };

    auto file = text::File{
        strings::String{"TestFile"},
        strings::String{"\x07 \x00"
                        R"(# Rebuild.Context.declareVariable hif :Rebuild.literal.String = "Hello from Global!"

Rebuild.Context.declareFunction(() hi (a :Rebuild.literal.String) ():
    # Rebuild.say hif # TODO(arBmind): get globals working
    Rebuild.say "Hello from Hi"
    Rebuild.say a
end
hi "Hello from calling"

Rebuild.Context.declareVariable foo :Rebuild.literal.String = "Hello from Variable!"
Rebuild.say foo
hi foo

Rebuild.Context.declareModule test:
    Rebuild.say "parsing inside!"
end
)"}};

    // std::cout << "\nTokens:\n";
    //// for (auto t : tokenize(file)) std::cout << t << '\n';

    // std::cout << "\nBlocks:\n";
    // std::cout << blockify(file);

    std::cout << "\nCompile:\n";
    compiler.compile(file);

    // std::cout << "Example = " << d.example() << '\n';
}
