#pragma once
#include "FileInput.h"
#include "Token.h"

#include "meta/Optional.h"

namespace scanner {

struct IdentifierScanner {
    static auto scan(FileInput& input) -> OptToken {
        if (!isStart(input)) return {};
        do {
            input.extend();
        } while (input.peek().map(isContinuation));
        return Token{IdentifierLiteral{input.range()}};
    }

private:
    static bool isStart(FileInput& input) {
        auto chr = input.peek().value();
        if (chr == '.') {
            auto optCp = input.peek<1>();
            if (!optCp) return false;
            chr = optCp.value();
            auto r = isFirst(chr);
            if (r) input.extend();
        }
        return isFirst(chr);
    }

    static bool isFirst(CodePoint cp) { return cp.isLetter() || cp.isPunctuationConnector(); }

    static bool isContinuation(CodePoint cp) {
        return cp.isLetter() || cp.isPunctuationConnector() || cp.isDecimalNumber();
    }
};

} // namespace scanner
