#include "Compiler.h"

#include "filter/filterTokens.h"
#include "nesting/nestTokens.h"
#include "parser/Parser.h"
#include "scanner/tokenize.h"
#include "strings/utf8Decode.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"
#include "intrinsic/ResolveType.h"

#include "diagnostic/Diagnostic.ostream.h"
#include "nesting/Token.ostream.h"
#include "scanner/Token.ostream.h"

#include <iostream>

namespace rec {

using InstanceNode = instance::Entry;
using ExecutionContext = execution::Context;
using StringView = strings::View;

using diagnostic::Diagnostic;
using nesting::BlockLiteral;
using parser::Block;
using parser::Call;
using parser::OptNode;

namespace {

struct IntrinsicType {
    const InstanceScope* globals;

    template<class T>
    auto operator()(meta::Type<T>) const -> instance::TypeView {
        return intrinsic::ResolveType<T>::template moduleInstance<intrinsic::Rebuild>(globals);
    }
};

auto assignResultStorage(Call& call) {
    const auto* f = call.function;
    for (auto* a : f->parameters) {
        // TODO(arBmind): new types - handle pointers
        //        if (a->side == instance::ParameterSide::result //
        //            && a->typed.type.holds<parser::Pointer>()) {

        //            parser::TypeExpression* ptrTarget = a->typed.type.get<parser::Pointer>().target.get();
        //            ptrTarget->visit(
        //                [&](const parser::Pointer& p) {
        //                    call.arguments.push_back({a, {parser::Value{nullptr,
        //                    parser::TypeExpression{*ptrTarget}}}});
        //                },
        //                [&](const parser::TypeInstance& i) {
        //                    call.arguments.push_back({a, {i.concrete->makeUninitialized({*ptrTarget})}});
        //                },
        //                [](const auto) {});
        //        }
    }
}

auto getResultValue(Call& call) -> meta::Optional<parser::Value> {
    auto result = meta::Optional<parser::Value>{};
    for (auto& a : call.arguments) {
        // TODO(arBmind): new types - extract results
        //        if (a.parameter->side == instance::ParameterSide::result //
        //            && a.parameter->typed.type.holds<parser::Pointer>()) {

        //            if (result || a.values.size() != 1 || !a.values[0].holds<parser::Value>()) return {};

        //            result = a.values[0].get<parser::Value>();
        //        }
    }
    return result;
}

auto extractResults(Call& call, const InstanceScope& globals) -> OptNode {
    auto optResult = getResultValue(call);
    if (optResult) {
        // TODO(arBmind): new types
        //        auto& resultType = optResult.value().type();
        //        if (resultType.holds<parser::TypeInstance>() &&
        //            resultType.get<parser::TypeInstance>().concrete ==
        //                IntrinsicType{&globals}(meta::Type<parser::VariableInit>{})) {

        //            return parser::Node{*reinterpret_cast<parser::VariableInit*>(optResult.value().data())};
        //        }
    }
    // TODO(arBmind): create TypedTuple from results
    return {};
}

} // namespace

auto Compiler::executionContext(InstanceScope& parserScope) {
    auto r = ExecutionContext{};
    r.compiler = &compilerCallback;
    r.parserScope = &parserScope;
    auto parse = [&](const BlockLiteral& blockLiteral, auto& parserContext) {
        return parser::Parser::parse(blockLiteral, parserContext);
    };
    auto declare = [&](InstanceNode&& node) { parserScope.emplace(std::move(node)); };
    (void)parse;
    (void)declare;
    return r;
}

auto Compiler::parserContext(InstanceScope& scope) {
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
    return parser::Context{std::move(lookup), std::move(runCall), IntrinsicType{&globals}, std::move(reportDiagnostic)};
}

Compiler::Compiler(Config config, InstanceScope _globals)
    : config(config)
    , globals(std::move(_globals))
    , globalScope(&globals) {

    globals.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    compilerCallback.parseBlock = [this](const BlockLiteral& block, InstanceScope* scope) -> parser::Block {
        return parser::Parser::parse(block, parserContext(*scope));
    };
    compilerCallback.reportDiagnostic = [this](Diagnostic diagnostic) {
        diagnostics.emplace_back(std::move(diagnostic));
    };
}

void Compiler::compile(const TextFile& file) {
    auto decode = [&](const auto& file) { return strings::utf8Decode(file.content); };
    auto positions = [&](const auto& file) { return text::decodePosition(decode(file), config); };
    auto tokenize = [&](const auto& file) { return scanner::tokenize(positions(file)); };
    auto filter = [&](const auto& file) { return filter::filterTokens(tokenize(file)); };
    auto blockify = [&](const auto& file) { return nesting::nestTokens(filter(file)); };
    auto parse = [&](const auto& file) { return parser::Parser::parse(blockify(file), parserContext(globalScope)); };

    if (config.tokenOutput) {
        auto& out = *config.tokenOutput;
        out << "\nTokens:\n";
        for (auto t : tokenize(file)) out << t << '\n';
    }
    if (config.blockOutput) {
        auto& out = *config.blockOutput;
        out << "\nBlocks:\n" << blockify(file);
    }

    auto block = parse(file);
    if (!diagnostics.empty()) {
        if (config.diagnosticsOutput) {
            auto& out = *config.diagnosticsOutput;
            out << diagnostics.size() << " diagnostics:\n";
            for (auto& d : diagnostics) out << d;
        }
    }
    else
        execution::Machine::runBlock(block, executionContext(globals));
}

} // namespace rec
