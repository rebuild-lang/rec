#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>
#include <text/DecodedPosition.h>

namespace scanner {

using text::CodePointPosition;
using text::DecodedPosition;

/** \brief parse a number literal from an enumerator
 *
 * key features:
 * * 0xA.BpF - hex float number with hex exponent
 * * 0o3.4p7 - octal float number with octal exponent
 * * 0b0.1p1 - binary float number with binary exponent
 * * 0.5e-99 - decimal float number
 * * decodeErrors are eaten
 * * one error is tracked
 */
inline auto extractNumber(CodePointPosition firstCpp, meta::CoEnumerator<DecodedPosition>& decoded) -> NumberLiteral {
    using text::CodePoint;
    using OptCodePointPosition = meta::Optional<CodePointPosition>;
    auto number = NumberLiteralValue{};

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
                number.decodeErrors.push_back(dep);
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
    auto nextCppWhile = [&](auto pred) -> OptCodePointPosition {
        auto optCpp = nextCpp();
        while (optCpp.map([&](auto cpp) { return pred(cpp.codePoint); })) {
            optCpp = nextCpp();
        }
        return optCpp;
    };

    static auto isP = [](CodePoint cp) { return cp.v == 'p' || cp.v == 'P'; };
    static auto isE = [](CodePoint cp) { return cp.v == 'e' || cp.v == 'E'; };
    static auto isPlus = [](CodePoint cp) { return cp.v == '+'; };
    static auto isMinus = [](CodePoint cp) { return cp.v == '-'; };
    static auto isDot = [](CodePoint cp) { return cp.v == '.'; };
    static auto isZero = [](CodePoint cp) { return cp.v == '0'; };
    //    static auto isDecimalZero = [](CodePoint cp) {
    //        return cp.decimalNumber() && [](auto decimal) { return decimal == strings::Decimal{0}; };
    //    };
    static auto isIgnored = [](CodePoint cp) { return cp.v == '\''; };
    static auto isZeroOrIgnored = [=](CodePoint cp) { return isZero(cp) || isIgnored(cp); };

    static auto isDecimalDigit = [](CodePoint cp) { return cp.isDecimalNumber(); };
    static auto isBinaryDigit = [](CodePoint cp) { return (cp.v >= '0' && cp.v <= '1'); };
    static auto isOctalDigit = [](CodePoint cp) { return (cp.v >= '0' && cp.v <= '7'); };
    static auto isHexDigit = [](CodePoint cp) {
        auto chr = cp.v;
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    };
    static auto isNoBoundary = [](CodePoint cp) { return cp.isDecimalNumber() || cp.isLetter(); };

    auto withRadix = [&](OptCodePointPosition optCpp, auto env) -> NumberLiteral {
        number.radix = env.radix();
        auto mapCp = [&](auto pred) -> bool {
            return optCpp.map([&](auto cpp) -> bool { return pred(cpp.codePoint); });
        };

        auto extendInto = [&](Rope& into) {
            while (mapCp(env.isDigit())) {
                into += optCpp.value().input;
                // optCpp nextCppWhile(isIgnored); // vs2017 15.9.2 crashes on this
                // workaround:
                optCpp = nextCpp();
                while (optCpp && optCpp.value().codePoint.v == '\'') optCpp = nextCpp();
            }
        };

        auto isIntegerStartZero = mapCp(isZero);
        if (isIntegerStartZero) optCpp = nextCppWhile(isZeroOrIgnored);
        extendInto(number.integerPart);
        if (mapCp(isDot)) {
            optCpp = nextCpp();
            extendInto(number.fractionalPart);
        }
        if (mapCp(env.isExponent())) {
            optCpp = nextCpp();
            if (mapCp(isPlus)) {
                number.exponentSign = Sign::positive;
                optCpp = nextCpp();
            }
            else if (mapCp(isMinus)) {
                number.exponentSign = Sign::negative;
                optCpp = nextCpp();
            }
            auto isExponentStartZero = mapCp(isZero);
            if (isExponentStartZero) optCpp = nextCppWhile(isZeroOrIgnored);

            extendInto(number.exponentPart);
            if (!isExponentStartZero && number.exponentPart.isEmpty()) {
                number.error = NumberLiteralError::MissingExponent;
            }
        }
        if (!isIntegerStartZero && number.integerPart.isEmpty() && number.fractionalPart.isEmpty()) {
            number.error = NumberLiteralError::MissingValue;
        }
        else if (mapCp(isNoBoundary)) {
            number.error = NumberLiteralError::MissingBoundary;
        }

        return {number, inputView(), firstCpp.position};
    };

    auto secondCpp = peekCpp();
    if (firstCpp.codePoint == '0' && secondCpp) {
        auto skipPrefix = [&]() -> OptCodePointPosition {
            isConsumed = false; // go to third!
            return nextCpp();
        };
        switch (secondCpp.value().codePoint.v) {
        case 'x':
        case 'X':
            struct Hex {
                static constexpr auto radix() { return Radix::hex; }
                static constexpr auto isDigit() { return isHexDigit; }
                static constexpr auto isExponent() { return isP; }
            };
            return withRadix(skipPrefix(), Hex{});

        case 'o':
        case 'O':
            struct Octal {
                static constexpr auto radix() { return Radix::octal; }
                static constexpr auto isDigit() { return isOctalDigit; }
                static constexpr auto isExponent() { return isP; }
            };
            return withRadix(skipPrefix(), Octal{});

        case 'b':
        case 'B':
            struct Binary {
                static constexpr auto radix() { return Radix::binary; }
                static constexpr auto isDigit() { return isBinaryDigit; }
                static constexpr auto isExponent() { return isP; }
            };
            return withRadix(skipPrefix(), Binary{});
        }
    }
    struct Decimal {
        static constexpr auto radix() { return Radix::decimal; }
        static constexpr auto isDigit() { return isDecimalDigit; }
        static constexpr auto isExponent() { return isE; }
    };
    return withRadix(firstCpp, Decimal{});
}

} // namespace scanner
