#pragma once
#include "extractComment.h"
#include "extractIdentifier.h"
#include "extractNumber.h"
#include "extractOperator.h"
#include "extractString.h"

#include <scanner/Token.h>

#include <meta/CoEnumerator.h>
#include <text/DecodedPosition.h>

namespace scanner {

using text::DecodedPosition;

template<class Token>
auto extractChar(CodePointPosition cpp) -> Token {
    return {cpp.input, cpp.position};
}

inline auto tokenize(meta::CoEnumerator<DecodedPosition> decoded) -> meta::CoEnumerator<Token> {
    using text::CodePointPosition;
    using text::DecodedErrorPosition;
    using text::NewlinePosition;
    using text::View;

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
                case ':': return extractChar<ColonSeparator>(cpp);
                case ',': return extractChar<CommaSeparator>(cpp);
                case ';': return extractChar<SemicolonSeparator>(cpp);
                case '[': return extractChar<SquareBracketOpen>(cpp);
                case ']': return extractChar<SquareBracketClose>(cpp);
                case '(': return extractChar<BracketOpen>(cpp);
                case ')': return extractChar<BracketClose>(cpp);
                }

                if (auto opt = extractIdentifier(cpp, decoded); opt) return opt.value();
                if (auto opt = extractOperator(cpp, decoded); opt) return opt.value();
                return UnexpectedCharacter{cpp.input, cpp.position};
            },
            [&](NewlinePosition nlp) -> Token {
                auto ws = extractWhitespaces(nlp);
                return NewLineIndentation{ws.input, ws.position};
            },
            [&](DecodedErrorPosition dep) -> Token {
                return InvalidEncoding{dep.input, dep.position};
            });
    }
}

} // namespace scanner
