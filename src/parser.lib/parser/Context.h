#pragma once
#include "parser/Tree.h"

#include "diagnostic/Diagnostic.h"
#include "instance/Node.h"

#include <type_traits>

namespace parser {

struct NoDiagnositics {
    auto operator()(diagnostic::Diagnostic&&) {}
};

template<class Lookup, class RunCall, class IntrinsicType, class ReportDiagnostic = NoDiagnositics>
struct Context {
    Lookup lookup; // strings::View -> instance::NodeView
    RunCall runCall; // Call -> AST Node
    IntrinsicType intrinsicType; // <Type> -> instance::TypeView
    ReportDiagnostic reportDiagnostic; // diagnostic::Diagnostic -> void
};

// template deduction guide
template<class Lookup, class RunCall, class IntrinsicType>
Context(Lookup&&, RunCall&&, IntrinsicType &&)->Context<Lookup, RunCall, IntrinsicType>;
template<class Lookup, class RunCall, class IntrinsicType, class ReportDiagnostic>
Context(Lookup&&, RunCall&&, IntrinsicType&&, ReportDiagnostic &&)
    ->Context<Lookup, RunCall, IntrinsicType, ReportDiagnostic>;

template<class Context>
struct ContextApi {
    static_assert(
        std::is_same_v<instance::OptConstNodeView, std::invoke_result_t<decltype(Context::lookup), strings::View>>,
        "no lookup");
    static_assert(
        std::is_same_v<OptNode, std::invoke_result_t<decltype(Context::runCall), Call>>, //
        "no runCall");
    static_assert(
        std::is_same_v<
            instance::TypeView,
            std::invoke_result_t<decltype(Context::intrinsicType), meta::Type<StringLiteral>>>,
        "no intrinsicType");
    static_assert(
        std::is_same_v<void, std::invoke_result_t<decltype(Context::reportDiagnostic), diagnostic::Diagnostic>>,
        "no reportDiagnostic");

    explicit ContextApi(Context context)
        : context(std::move(context)) {}

    auto lookup(strings::View view) const -> instance::OptConstNodeView { return context.lookup(view); }
    auto runCall(Call call) const -> OptNode { return context.runCall(std::move(call)); }

    template<class Type>
    auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
        return context.intrinsicType(meta::Type<Type>{});
    }
    auto reportDiagnostic(diagnostic::Diagnostic diagnostic) -> void {
        return context.reportDiagnostic(std::move(diagnostic));
    }

private:
    Context context;
};

} // namespace parser
