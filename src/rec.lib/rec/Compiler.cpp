#include "Compiler.h"

namespace rec {

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

auto getResultValue(Call& call) -> meta::Optional<parser::Value> {
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

auto extractResults(Call& call, const InstanceScope& globals) -> OptNode {
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

void Compiler::printDiagnostics() const {
    std::cout << diagnostics.size() << " diagnostics created\n";
    for (auto& d : diagnostics) std::cout << d;
}

Compiler::Compiler(ParserConfig config, InstanceScope&& globals)
    : config(config)
    , globals(std::move(globals))
    , globalScope(&this->globals) {
    compilerCallback.parseBlock = [this](const BlockLiteral& block, InstanceScope* scope) -> parser::Block {
        return parser::Parser::parse(block, parserContext(*scope));
    };
}

void Compiler::compile(const text::File& file) {
    auto decode = [&](const auto& file) { return strings::utf8Decode(file.content); };
    auto positions = [&](const auto& file) { return text::decodePosition(decode(file), config); };
    auto tokenize = [&](const auto& file) { return scanner::tokenize(positions(file)); };
    auto filter = [&](const auto& file) { return filter::filterTokens(tokenize(file)); };
    auto blockify = [&](const auto& file) { return nesting::nestTokens(filter(file)); };
    auto parse = [&](const auto& file) { return parser::Parser::parse(blockify(file), parserContext(globalScope)); };
    auto block = parse(file);
    if (!diagnostics.empty())
        printDiagnostics();
    else
        execution::Machine::runBlock(block, executionContext(globals));
}

} // namespace rec
