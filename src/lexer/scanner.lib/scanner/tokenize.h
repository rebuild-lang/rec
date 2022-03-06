#pragma once
#include "extractComment.h"
#include "extractIdentifier.h"
#include "extractNewLineIndentation.h"
#include "extractNumber.h"
#include "extractOperator.h"
#include "extractString.h"

#include <scanner/Token.h>

#include <meta/CoEnumerator.h>
#include <meta/Type.h>
#include <text/DecodedPosition.h>

namespace scanner {

using meta::CoEnumerator;
using meta::type;
using meta::Type;
using text::DecodedPosition;

inline auto extractWhitespaces(CodePointPosition firstCpp, CoEnumerator<DecodedPosition>& decoded)
    -> WhiteSpaceSeparator {
    auto const input = firstCpp.input;
    auto const begin = input.begin();
    auto end = input.end();
    while (decoded) {
        if (decoded->holds<CodePointPosition>()) {
            auto cpp = decoded->get<CodePointPosition>();
            if (cpp.codePoint.isWhiteSpace()) {
                end = cpp.input.end();
                decoded++;
                continue;
            }
        }
        break;
    }
    return WhiteSpaceSeparator{{View{begin, end}, firstCpp.position}};
};

inline auto tokenize(CoEnumerator<DecodedPosition> decoded) -> CoEnumerator<Token> {
    using text::CodePointPosition;
    using text::DecodedErrorPosition;
    using text::NewlinePosition;
    using text::View;

    static constexpr auto extractChar = []<class Token>(Type<Token>, CodePointPosition cpp) -> Token {
        return {{cpp.input, cpp.position}};
    };

    auto newLineState = ExtractNewLineState{};

    auto handleCpp = [&decoded](CodePointPosition cpp) -> Token {
        auto chr = cpp.codePoint;

        if (chr.isWhiteSpace()) return extractWhitespaces(cpp, decoded);
        if (chr.isDecimalNumber()) return extractNumber(cpp, decoded);

        switch (chr.v) {
        case '"': return extractString(cpp, decoded);
        case '#': return extractComment(cpp, decoded);
        case ':': return extractChar(type<ColonSeparator>, cpp);
        case ',': return extractChar(type<CommaSeparator>, cpp);
        case ';': return extractChar(type<SemicolonSeparator>, cpp);
        case '[': return extractChar(type<SquareBracketOpen>, cpp);
        case ']': return extractChar(type<SquareBracketClose>, cpp);
        case '(': return extractChar(type<BracketOpen>, cpp);
        case ')': return extractChar(type<BracketClose>, cpp);
        }

        if (auto opt = extractIdentifier(cpp, decoded); opt) return opt.value();
        if (auto opt = extractOperator(cpp, decoded); opt) return opt.value();
        return UnexpectedCharacter{{cpp.input, cpp.position}};
    };

    ++decoded;
    while (decoded) {
        auto current = *decoded;
        ++decoded;
        if (current.holds<CodePointPosition>()) {
            co_yield handleCpp(current.get<CodePointPosition>());
        }
        else if (current.holds<NewlinePosition>()) {
            co_yield extractNewLineIndentation(current.get<NewlinePosition>(), decoded, newLineState);
        }
        else {
            auto [input, position] = current.get<DecodedErrorPosition>();
            co_yield InvalidEncoding{{input, position}};
        }
        // MSVC2022 17.1.0 bugs out on this
        // co_yield current.visit(
        //     [&](CodePointPosition cpp) -> Token { retrun handleCpp(cpp); },
        //     [&](NewlinePosition nlp) -> Token { return extractNewLineIndentation(nlp, decoded, newLineState); },
        //     [&](DecodedErrorPosition dep) -> Token { return InvalidEncoding{dep.input, dep.position}; });
    }
}

} // namespace scanner
