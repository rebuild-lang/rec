#pragma once
#include "extractNumber.h"

#include <scanner/Token.h>

#include <meta/CoEnumerator.h>
#include <text/DecodedPosition.h>

namespace scanner {

using text::DecodedPosition;

inline auto tokenize(meta::CoEnumerator<DecodedPosition> decoded) -> meta::CoEnumerator<Token> {
    using text::CodePointPosition;
    using text::DecodedErrorPosition;
    using text::NewlinePosition;
    using text::View;

    auto scanWhitespaces = [&](View p) {
        auto end = p.end();
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
        return end;
    };
    auto whitespace = [&](CodePointPosition cpp) {
        auto end = scanWhitespaces(cpp.input);
        return WhiteSpaceSeparator{View{cpp.input.begin(), end}, cpp.position};
    };
    auto numberLiteral = [&](CodePointPosition cpp) { return extractNumber(cpp, decoded); };
    // auto stringLiteral = [&](CodePointPosition) { return extractString(cpp, decoded); };
    auto newLineIndentation = [&](NewlinePosition nlp) {
        auto end = scanWhitespaces(nlp.input);
        return NewLineIndentation{View{nlp.input.begin(), end}, nlp.position};
    };
    auto unexpected = [&](auto dep) { return UnexpectedCharacter{dep.input, dep.position}; };

    decoded++;
    while (decoded) {
        auto current = *decoded;
        decoded++;
        co_yield current.visit(
            [&](CodePointPosition cpp) -> Token {
                auto chr = cpp.codePoint;
                if (chr.isWhiteSpace()) return whitespace(cpp);
                if (chr.isDecimalNumber()) return numberLiteral(cpp);

                /*switch (chr.v) {
                case '"':
                    // return stringLiteral(cpp);
                    // …
                }*/
                return unexpected(cpp);
            },
            [&](NewlinePosition nlp) -> Token { return newLineIndentation(nlp); },
            [&](DecodedErrorPosition dep) -> Token { return unexpected(dep); });
    }
}

} // namespace scanner
