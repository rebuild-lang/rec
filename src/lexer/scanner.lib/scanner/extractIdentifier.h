#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>

namespace scanner {

using text::CodePointPosition;
using text::DecodedPosition;

inline auto extractIdentifier(CodePointPosition firstCpp, meta::CoEnumerator<DecodedPosition>& decoded) -> OptToken {
    using OptCodePointPosition = meta::Optional<CodePointPosition>;
    using text::CodePoint;

    auto end = firstCpp.input.end();
    auto inputView = [&, begin = firstCpp.input.begin()] { return View{begin, end}; };
    auto errors = IdentifierLiteralErrors{};

    auto peekCpp = [&]() -> OptCodePointPosition {
        while (true) {
            if (!decoded) return {};
            auto dp = *decoded;
            if (dp.holds<CodePointPosition>()) {
                return dp.get<CodePointPosition>();
            }
            if (dp.holds<DecodedErrorPosition>()) {
                auto dep = dp.get<DecodedErrorPosition>();
                errors.emplace_back(dep);
                end = dep.input.end();
                decoded++;
                continue;
            }
            return {};
        }
    };
    auto nextCpp = [&]() -> OptCodePointPosition {
        end = (*decoded).get<CodePointPosition>().input.end();
        decoded++;
        return peekCpp();
    };

    auto isFirst = [](CodePoint cp) -> bool { return cp.isLetter() || cp.isPunctuationConnector(); };
    auto isContinuation = [](CodePointPosition cpp) -> bool {
        auto cp = cpp.codePoint;
        return cp.isLetter() || cp.isPunctuationConnector() || cp.isDecimalNumber();
    };

    auto type = IdentifierLiteralType::normal;
    auto isStart = [&]() -> bool {
        auto chr = firstCpp.codePoint;
        if (chr == '.') {
            auto optCpp = peekCpp();
            if (!optCpp) return false;
            chr = optCpp.value().codePoint;
            auto r = isFirst(chr);
            if (r) nextCpp();
            type = IdentifierLiteralType::member;
        }
        return isFirst(chr);
    };

    if (!isStart()) return {};
    auto optCpp = peekCpp();
    while (optCpp.map(isContinuation)) optCpp = nextCpp();
    return Token{IdentifierLiteral{{inputView(), firstCpp.position}, {type, std::move(errors)}}};
}

} // namespace scanner
