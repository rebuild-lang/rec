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

using meta::type;
using text::DecodedPosition;

inline auto tokenize(meta::CoEnumerator<DecodedPosition> decoded) -> meta::CoEnumerator<Token> {
    using text::CodePointPosition;
    using text::DecodedErrorPosition;
    using text::NewlinePosition;
    using text::View;

    auto extractChar = [](auto token, CodePointPosition cpp) -> meta::ToBareType<decltype(token)> {
        return {cpp.input, cpp.position};
    };

    auto extractWhitespaces = [&](auto first) {
        auto end = first.input.end();
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
        return WhiteSpaceSeparator{View{first.input.begin(), end}, first.position};
    };

    auto newLineState = ExtractNewLineState{};

    decoded++;
    while (decoded) {
        auto current = *decoded;
        decoded++;
        co_yield current.visit(
            [&](CodePointPosition cpp) -> Token {
                auto chr = cpp.codePoint;
                if (chr.isWhiteSpace()) return extractWhitespaces(cpp);
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
                return UnexpectedCharacter{cpp.input, cpp.position};
            },
            [&](NewlinePosition nlp) -> Token { return extractNewLineIndentation(nlp, decoded, newLineState); },
            [&](DecodedErrorPosition dep) -> Token {
                return InvalidEncoding{dep.input, dep.position};
            });
    }
}

} // namespace scanner
