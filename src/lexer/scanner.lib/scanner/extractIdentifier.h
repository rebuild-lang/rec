#pragma once
#include "scanner/Token.h"

#include "text/FileInput.h"

#include "meta/Optional.h"

namespace scanner {

inline auto extractIdentifier(text::FileInput& input) -> OptToken {
    auto isFirst = [](text::CodePoint cp) -> bool { return cp.isLetter() || cp.isPunctuationConnector(); };
    auto isContinuation = [](text::CodePoint cp) -> bool {
        return cp.isLetter() || cp.isPunctuationConnector() || cp.isDecimalNumber();
    };

    auto isStart = [&]() -> bool {
        auto chr = input.peek().value();
        if (chr == '.') {
            auto optCp = input.peek<1>();
            if (!optCp) return false;
            chr = optCp.value();
            auto r = isFirst(chr);
            if (r) input.extend();
        }
        return isFirst(chr);
    };

    if (!isStart()) return {};
    do {
        input.extend();
    } while (input.peek().map(isContinuation));
    // return Token{IdentifierLiteral{input.range()}};
    return Token{};
}

} // namespace scanner
