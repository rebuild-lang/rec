#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>

#include <stack>

namespace scanner {

using text::CodePointPosition;
using text::DecodedPosition;
using StringIterator = strings::View::It;

inline auto extractOperator(CodePointPosition firstCpp, meta::CoEnumerator<DecodedPosition>& decoded) -> OptToken {
    using OptCodePointPosition = meta::Optional<CodePointPosition>;
    using strings::CodePoint;
    using strings::OptionalCodePoint;
    using text::Position;
    struct Entry {
        CodePoint closeCp;
        StringIterator begin{};
        Position beginPosition;
    };
    using Stack = std::vector<Entry>;
    auto stack = Stack{};
    auto decodeErrors = DecodedErrorPositions{};

    auto isConsumed = true;
    auto end = firstCpp.input.end();
    auto inputView = [&, begin = firstCpp.input.begin()] { return View{begin, end}; };

    auto peekCpp = [&]() -> OptCodePointPosition {
        while (true) {
            if (!decoded) return {};
            auto dp = *decoded;
            if (dp.holds<CodePointPosition>()) {
                return dp.get<CodePointPosition>();
            }
            if (dp.holds<DecodedErrorPosition>()) {
                auto dep = dp.get<DecodedErrorPosition>();
                decodeErrors.push_back(dep);
                end = dep.input.end();
                decoded++;
                continue;
            }
            return {};
        }
    };
    auto nextCpp = [&]() -> OptCodePointPosition {
        if (!isConsumed) {
            end = (*decoded).get<CodePointPosition>().input.end();
            decoded++;
        }
        isConsumed = false;
        return peekCpp();
    };
    auto nextCppWhile = [&](auto pred) {
        auto optCpp = nextCpp();
        while (optCpp.map(pred)) optCpp = nextCpp();
    };

    auto closePunctuation = [](CodePoint cp) -> OptionalCodePoint {
        // https://www.unicode.org/charts/script/chart_Punctuation-Open.html
        // <-> https://www.unicode.org/charts/script/chart_Punctuation-Close.html
        switch (cp.v) {
        // case '(': return CodePoint{')'}; // note: handled before
        // case '[': return CodePoint{']'}; // note: handled before
        case '{': return CodePoint{'}'};
        case 0x2045: return CodePoint{0x2046}; // ⁅ - ⁆
        case 0x2308: return CodePoint{0x2309}; // ⌈ - ⌉
        case 0x230A: return CodePoint{0x230B}; // ⌊ - ⌋
        case 0x276A: return CodePoint{0x276B}; // ❪ - ❫
        case 0x276C: return CodePoint{0x276D}; // ❬ - ❭
        case 0x276E: return CodePoint{0x276F}; // ❮ - ❯
        case 0x27C5: return CodePoint{0x27C6}; // ⟅ - ⟆
        case 0x27E6: return CodePoint{0x27E7}; // ⟦ - ⟧
        case 0x27E8: return CodePoint{0x27E9}; // ⟨ - ⟩
        case 0x27EA: return CodePoint{0x27EB}; // ⟪ - ⟫
        case 0x27EC: return CodePoint{0x27ED}; // ⟬ - ⟭
        case 0x27EE: return CodePoint{0x27EF}; // ⟮ - ⟯
        case 0x2983: return CodePoint{0x2984}; // ⦃ - ⦄
        case 0xFF5F: return CodePoint{0xFF60}; // ⦅ - ｠
        case 0x2987: return CodePoint{0x2988}; // ⦇ - ⦈
        case 0x2989: return CodePoint{0x298A}; // ⦉ - ⦊
        case 0x2991: return CodePoint{0x2992}; // ⦑ - ⦒
        case 0x2993: return CodePoint{0x2994}; // ⦓ - ⦔
        case 0x2995: return CodePoint{0x2996}; // ⦕ - ⦖
        case 0x29FC: return CodePoint{0x29FD}; // ⧼ - ⧽
        }
        // https://www.unicode.org/charts/script/chart_Punctuation-Initial.html
        // <-> https://www.unicode.org/charts/script/chart_Punctuation-Final.html
        switch (cp.v) {
        case 0x00AB: return CodePoint{0x00BB}; // « - »
        case 0x2018: return CodePoint{0x2019}; // ‘ - ’
        case 0x201C: return CodePoint{0x201D}; // “ - ”
        case 0x2039: return CodePoint{0x203A}; // ‹ - ›
        case 0x2E02: return CodePoint{0x2E03}; // ⸂ - ⸃
        case 0x2E04: return CodePoint{0x2E05}; // ⸄ - ⸅
        case 0x2E20: return CodePoint{0x2E21}; // ⸠ - ⸡
        }
        return {};
    };

    auto isEnclosedPart = [&](CodePointPosition cpp) -> bool {
        auto cp = cpp.codePoint;
        if (cp.isWhiteSpace()) return false;
        if (stack.back().closeCp == cp) {
            stack.pop_back();
            return !stack.empty();
        }
        auto cpu = closePunctuation(cp);
        if (cpu) {
            stack.push_back(Entry{cpu.value(), cpp.input.begin(), cpp.position});
            return true;
        }
        return true;
    };

    auto isPart = [&](CodePointPosition cpp) -> bool {
        auto cp = cpp.codePoint;
        if (cp == '-' || cp.isSymbolMath() || cp.isSymbolOther() || cp.isNumberOther()) return true;
        if (cp.isSymbolCurrency() && cp.v != '$') return true;
        if (cp.isPunctuationOther() && cp.v != '.') return true; // others are handled before
        auto cpu = closePunctuation(cp);
        if (cpu) {
            stack.push_back(Entry{cpu.value(), cpp.input.begin(), cpp.position});
            nextCppWhile(isEnclosedPart);
            return stack.empty();
        }
        return false;
    };

    if (!isPart(firstCpp)) return {};
    nextCppWhile(isPart);
    if (!stack.empty()) {
        // error!
    }
    return Token{OperatorLiteral{inputView(), firstCpp.position, decodeErrors}};
}

} // namespace scanner
