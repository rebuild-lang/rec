#pragma once
#include "TupleLookup.h"

#include "parser/Expression.h"

#include "diagnostic/Diagnostic.h"
#include "instance/Entry.h"

#include <type_traits>

namespace parser {

using diagnostic::Diagnostic;

// Context used to parse a block
template<class Base>
struct Context {
private:
    auto base() const noexcept -> const Base& { return static_cast<const Base&>(*this); }
    auto base() noexcept -> Base& { return static_cast<Base&>(*this); }

public:
    using BaseContext = Base;

    // lookup an identifier in the scope of current context
    // should find results from first parent scope
    [[nodiscard]] auto lookup(strings::View view) const -> instance::ConstEntryRange {
        return base().lookup(view); //
    }

    // executes a fully known call to a function and returns the result
    [[nodiscard]] auto runCall(Call call) const -> OptValueExpr {
        return base().runCall(std::move(call)); //
    }

    // resolve a C++ type to the builtin rebuild type
    // mainly used to store literal types
    template<class Type>
    auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
        return base().intrinsicType(meta::type<Type>);
    }

    // report a diagnostic to the user
    void reportDiagnostic(Diagnostic diagnostic) {
        return base().reportDiagnostic(std::move(diagnostic)); //
    }
};

template<class T>
constexpr auto is_context = std::is_base_of_v<Context<T>, T>;

// enhanced context to parse a tuple with cross references
template<class BC>
struct ContextWithTupleLookup : BC {
private:
    static_assert(is_context<BC>);
    using This = ContextWithTupleLookup;
    TupleLookup tupleLookup{};

public:
    using BaseContext = BC;

    explicit ContextWithTupleLookup(BaseContext base, const NameTypeValueTuple* tuple)
        : BaseContext(std::move(base))
        , tupleLookup(tuple) {}
    explicit ContextWithTupleLookup(const This& base, const NameTypeValueTuple* tuple)
        : BaseContext(std::move(base))
        , tupleLookup(tuple, &base.tupleLookup) {}

    [[nodiscard]] auto lookupTuple(strings::View view) const -> OptNameTypeValueView {
        return tupleLookup.byName(view);
    }
};

struct NoDiagnositics {
    auto operator()(Diagnostic&&) {}
};

template<class Lookup, class RunCall, class IntrinsicType, class ReportDiagnostic = NoDiagnositics>
struct ComposeContext : Context<ComposeContext<Lookup, RunCall, IntrinsicType, ReportDiagnostic>> {
    Lookup lookup; // strings::View -> instance::ConstEntryRange
    RunCall runCall; // Call -> OptValueExpr
    IntrinsicType intrinsicType; // <Type> -> instance::TypeView
    ReportDiagnostic reportDiagnostic; // diagnostic::Diagnostic -> void

    ComposeContext(
        Lookup&& lookup, RunCall&& runCall, IntrinsicType&& intrinsicType, ReportDiagnostic&& reportDiagnostic = {})
        : lookup(std::move(lookup))
        , runCall(std::move(runCall))
        , intrinsicType(std::move(intrinsicType))
        , reportDiagnostic(std::move(reportDiagnostic)) {}
};

// template deduction guide
template<class Lookup, class RunCall, class IntrinsicType>
ComposeContext(Lookup&&, RunCall&&, IntrinsicType &&)->ComposeContext<Lookup, RunCall, IntrinsicType>;
template<class Lookup, class RunCall, class IntrinsicType, class ReportDiagnostic>
ComposeContext(Lookup&&, RunCall&&, IntrinsicType&&, ReportDiagnostic &&)
    ->ComposeContext<Lookup, RunCall, IntrinsicType, ReportDiagnostic>;

} // namespace parser
