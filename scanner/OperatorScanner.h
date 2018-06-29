#pragma once
#include "FileInput.h"
#include "Token.h"

#include <stack>

namespace scanner {

struct OperatorScanner {
    static auto scan(FileInput& input) -> OptToken {
        auto stack = Stack{};
        auto isValid = [&] { //
            return input.peek().map([&](CodePoint cp) { return isPart(cp, input, stack); });
        };
        if (!isValid()) return {};
        do {
            input.extend();
        } while (isValid());
        if (!stack.empty()) {
            input.restoreCurrent(stack.front().it, stack.front().itPosition);
            if (input.view().isEmpty()) return {};
        }
        return Token{OperatorLiteral{input.range()}};
    }

private:
    struct Entry {
        CodePoint closeCp;
        FileInput::StringIterator it{};
        Position itPosition;
    };
    using Stack = std::vector<Entry>;

    static void scanEnclosed(FileInput& input, Stack& stack) {
        auto isValid = [&] { //
            return input.peek().map([&](CodePoint cp) { return isEnclosedPart(cp, input, stack); });
        };
        do {
            input.extend();
        } while (isValid());
    }

    static bool isPart(CodePoint cp, FileInput& input, Stack& stack) {
        if (cp == '-' || cp.isSymbolMath() || cp.isSymbolOther() || cp.isNumberOther()) return true;
        if (cp.isSymbolCurrency() && cp.v != '$') return true;
        if (cp.isPunctuationOther() && cp.v != '.') return true; // others are handled before
        auto cpu = closePunctuation(cp);
        if (cpu) {
            stack.push_back({cpu.value(), input.current(), input.currentPosition()});
            scanEnclosed(input, stack);
            return true;
        }
        return false;
    }

    static bool isEnclosedPart(CodePoint cp, FileInput& input, Stack& stack) {
        if (cp.isWhiteSpace() || cp.isLineSeparator()) return false;
        if (stack.back().closeCp == cp) {
            stack.pop_back();
            return !stack.empty();
        }
        auto cpu = closePunctuation(cp);
        if (cpu) {
            stack.push_back({cpu.value(), input.current(), input.currentPosition()});
            return true;
        }
        return true;
    }

    static auto closePunctuation(CodePoint cp) -> OptCodePoint {
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
    }
};

} // namespace scanner
