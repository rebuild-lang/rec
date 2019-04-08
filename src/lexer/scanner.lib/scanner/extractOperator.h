#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>
#include <meta/TypePack.h>

#include <stack>

namespace scanner {

namespace details {

using meta::TypePack;

template<uint32_t open, uint32_t close>
struct OpenClosePunctuation {};

template<uint32_t open, uint32_t close>
constexpr auto getOpen(OpenClosePunctuation<open, close>) {
    return open;
}
template<uint32_t open, uint32_t close>
constexpr auto getClose(OpenClosePunctuation<open, close>) {
    return close;
}

using OpenClosePunctuations = TypePack<
    // https://www.unicode.org/charts/script/chart_Punctuation-Open.html
    // <-> https://www.unicode.org/charts/script/chart_Punctuation-Close.html
    // OpenClosePunctuation<'(', ')'>, // note: handled before
    // OpenClosePunctuation<'[', ']'>, // note: handled before
    OpenClosePunctuation<'{', '}'>,
    OpenClosePunctuation<0x2045, 0x2046>, // ⁅ - ⁆
    OpenClosePunctuation<0x2308, 0x2309>, // ⌈ - ⌉
    OpenClosePunctuation<0x230A, 0x230B>, // ⌊ - ⌋
    OpenClosePunctuation<0x276A, 0x276B>, // ❪ - ❫
    OpenClosePunctuation<0x276C, 0x276D>, // ❬ - ❭
    OpenClosePunctuation<0x276E, 0x276F>, // ❮ - ❯
    OpenClosePunctuation<0x27C5, 0x27C6>, // ⟅ - ⟆
    OpenClosePunctuation<0x27E6, 0x27E7>, // ⟦ - ⟧
    OpenClosePunctuation<0x27E8, 0x27E9>, // ⟨ - ⟩
    OpenClosePunctuation<0x27EA, 0x27EB>, // ⟪ - ⟫
    OpenClosePunctuation<0x27EC, 0x27ED>, // ⟬ - ⟭
    OpenClosePunctuation<0x27EE, 0x27EF>, // ⟮ - ⟯
    OpenClosePunctuation<0x2983, 0x2984>, // ⦃ - ⦄
    OpenClosePunctuation<0xFF5F, 0xFF60>, // ⦅ - ｠
    OpenClosePunctuation<0x2987, 0x2988>, // ⦇ - ⦈
    OpenClosePunctuation<0x2989, 0x298A>, // ⦉ - ⦊
    OpenClosePunctuation<0x2991, 0x2992>, // ⦑ - ⦒
    OpenClosePunctuation<0x2993, 0x2994>, // ⦓ - ⦔
    OpenClosePunctuation<0x2995, 0x2996>, // ⦕ - ⦖
    OpenClosePunctuation<0x29FC, 0x29FD>, // ⧼ - ⧽
    // https://www.unicode.org/charts/script/chart_Punctuation-Initial.html
    // <-> https://www.unicode.org/charts/script/chart_Punctuation-Final.html
    OpenClosePunctuation<0x00AB, 0x00BB>, // « - »
    OpenClosePunctuation<0x2018, 0x2019>, // ‘ - ’
    OpenClosePunctuation<0x201C, 0x201D>, // “ - ”
    OpenClosePunctuation<0x2039, 0x203A>, // ‹ - ›
    OpenClosePunctuation<0x2E02, 0x2E03>, // ⸂ - ⸃
    OpenClosePunctuation<0x2E04, 0x2E05>, // ⸄ - ⸅
    OpenClosePunctuation<0x2E20, 0x2E21> // ⸠ - ⸡
    >;

using strings::CodePoint;
using strings::OptionalCodePoint;

template<class... OCPs>
auto getCloseCodePoint(CodePoint cp, TypePack<OCPs...>) -> OptionalCodePoint {
    OptionalCodePoint r{};
    ((cp.v == getOpen(OCPs{}) ? ((r = CodePoint{getClose(OCPs{})}), 0) : 0), ...);
    return r;
}

template<class... OCPs>
auto isCloseCodePoint(CodePoint cp, TypePack<OCPs...>) {
    return ((cp.v == getClose(OCPs{})) || ...);
}

} // namespace details

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
    auto errors = OperatorLiteralErrors{};

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
                errors.emplace_back(dep);
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
        return details::getCloseCodePoint(cp, details::OpenClosePunctuations{});
    };
    auto isClosePunctuation = [](CodePoint cp) -> bool {
        return details::isCloseCodePoint(cp, details::OpenClosePunctuations{});
    };

    auto isEnclosedPart = [&](CodePointPosition cpp) -> bool {
        auto cp = cpp.codePoint;
        if (cp.isWhiteSpace() || cp.isControl() || cp.isLineSeparator()) return false;
        if (isClosePunctuation(cp)) {
            if (stack.back().closeCp == cp) {
                stack.pop_back();
                return !stack.empty();
            }
            errors.emplace_back(OperatorWrongClose{{View{stack.back().begin, cpp.input.end()}, cpp.position}});
            return true;
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
        if (isClosePunctuation(cp)) {
            errors.emplace_back(OperatorUnexpectedClose{{cpp.input, cpp.position}});
            return true;
        }
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
        errors.emplace_back(OperatorNotClosed{{View{stack.back().begin, end}, stack.back().beginPosition}});
    }
    return Token{OperatorLiteral{{inputView(), firstCpp.position}, {std::move(errors)}}};
} // namespace scanner

} // namespace scanner
