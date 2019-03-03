#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>

namespace scanner {

using text::CodePointPosition;
using text::DecodedPosition;
using text::NewlinePosition;

struct ExtractNewLineState {
    text::CodePoint codePoint{};
};

inline auto extractNewLineIndentation(
    NewlinePosition nlp, //
    meta::CoEnumerator<DecodedPosition>& decoded,
    ExtractNewLineState& state) -> NewLineIndentation {

    auto newLine = NewLineIndentationValue{};
    auto end = nlp.input.end();
    auto isMixed = false;
    while (decoded) {
        if (decoded->holds<CodePointPosition>()) {
            auto cpp = decoded->get<CodePointPosition>();
            if (cpp.codePoint.isWhiteSpace() || cpp.codePoint.isTab()) {
                if (!state.codePoint) {
                    state.codePoint = cpp.codePoint;
                }
                else if (!isMixed && cpp.codePoint != state.codePoint) {
                    newLine.errors.emplace_back(MixedIndentCharacter{cpp.input, cpp.position});
                    isMixed = true;
                }
                end = cpp.input.end();
                newLine.indentColumn = cpp.endPosition.column;
                decoded++;
                continue;
            }
        }
        else if (decoded->holds<DecodedErrorPosition>()) {
            auto dep = decoded->get<DecodedErrorPosition>();
            newLine.errors.push_back(dep);
            decoded++;
            continue;
        }
        break;
    }
    return NewLineIndentation{View{nlp.input.begin(), end}, nlp.position, newLine};
}

} // namespace scanner
