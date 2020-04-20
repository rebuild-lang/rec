#pragma once
#include "LineErrorReporter.h"
#include "LineView.h"

#include "diagnostic/Diagnostic.h"

#include <type_traits>

namespace parser {

using nesting::BlockLine;
using nesting::Token;

struct CallErrorReporter {
    template<class T>
    struct External {
        T api;

        static_assert(
            std::is_same_v<
                instance::TypeView,
                decltype(std::declval<T>().intrinsicType(std::declval<meta::Type<StringLiteral>>()))>,
            "no intrinsicType");
        static_assert(
            std::is_same_v<void, decltype(std::declval<T>().reportDiagnostic(std::declval<diagnostic::Diagnostic>()))>,
            "no reportDiagnostic");
        static_assert(
            std::is_same_v<
                OptValueExpr,
                decltype( //
                    std::declval<T>().parserForType(std::declval<const TypeView&>()) //
                    (std::declval<BlockLineView&>()))>,
            "no parserForType");
        static_assert(
            std::is_same_v<
                OptNameTypeValue,
                decltype(std::declval<T>().parseNtvWithCallback(
                    std::declval<BlockLineView&>(), std::declval<void(NameTypeValue&)>()))>,
            "no parseNtvWithCallback");

        template<class Type>
        auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
            return api.intrinsicType(meta::type<Type>);
        }

        void reportDiagnostic(diagnostic::Diagnostic diagnostic) { api.reportDiagnostic(std::move(diagnostic)); }
        auto parserForType(const TypeView& type) { return api.parserForType(type); }

        template<class Callback>
        auto parseNtvWithCallback(BlockLineView& it, Callback&& cb) -> OptNameTypeValue {
            return api.parseNtvWithCallback(it, std::forward<Callback>(cb));
        }
    };

    template<class T>
    static void reportMissingClosingBracket(
        const nesting::BracketOpen& open, const BlockLineView& it, External<T>& external) {
        using namespace diagnostic;

        auto* blockLine = it.line();
        auto source = extractBlockLines(*blockLine);
        auto viewMarkers = ViewMarkers{};
        viewMarkers.emplace_back(open.input);
        auto line = open.position.line;
        if (it) {
            auto missingInput = it.current().visit([](auto& t) { return t.input; });
            line = it.current().visit([](auto& t) { return t.position.line; });
            viewMarkers.emplace_back(View{missingInput.begin(), missingInput.begin() + 1});
        }

        auto [escapedLines, escapedMarkers] = escapeSourceLine(source, viewMarkers);

        auto highlights = Highlights{};
        for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

        auto doc = Document{
            {Paragraph{
                 (viewMarkers.size() == 1)
                     ? String{"A call with opening bracket is expected to close before the end of the line."}
                     : String{"An closing bracket for the call was expected here."},
                 {}},
             SourceCodeBlock{escapedLines, highlights, String{}, line}}};

        auto expl = Explanation{String("Missing Closing Bracket"), doc};

        auto d = Diagnostic{Code{String{"rebuild-call"}, 1}, Parts{expl}};
        external.reportDiagnostic(std::move(d));
    }
};

} // namespace parser
