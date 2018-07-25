#pragma once
#include "scanner/Token.h"

#include "text/FileInput.h"

namespace scanner {

using CodePoint = strings::CodePoint;

inline auto extractNumber(CodePoint chr, text::FileInput& input) -> NumberLiteral {
    auto extendWhile = [&](auto pred) -> text::OptCodePoint {
        while (true) {
            input.extend();
            auto chr = input.peek();
            if (chr && pred(chr.value())) continue;
            return chr;
        }
    };

    auto isP = [](CodePoint cp) { return cp.v == 'p' || cp.v == 'P'; };
    auto isE = [](CodePoint cp) { return cp.v == 'e' || cp.v == 'E'; };
    auto isPlus = [](CodePoint cp) { return cp.v == '+'; };
    auto isMinus = [](CodePoint cp) { return cp.v == '-'; };
    auto isDot = [](CodePoint cp) { return cp.v == '.'; };
    auto isZero = [](CodePoint cp) { return cp.v == '0'; };
    auto isDecimalZero = [](CodePoint cp) {
        return cp.decimalNumber() && [](auto decimal) { return decimal == strings::Decimal{0}; };
    };
    auto isIgnored = [](CodePoint cp) { return cp.v == '\''; };
    auto isZeroOrIgnored = [=](CodePoint cp) { return isZero(cp) || isIgnored(cp); };

    auto isDecimalDigit = [](CodePoint cp) { return cp.isDecimalNumber(); };
    auto isBinaryDigit = [](CodePoint cp) { return (cp.v >= '0' && cp.v <= '1'); };
    auto isOctalDigit = [](CodePoint cp) { return (cp.v >= '0' && cp.v <= '7'); };
    auto isHexDigit = [](CodePoint cp) {
        auto chr = cp.v;
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    };

    auto withRadix = [&](Radix radix, auto isDigit, auto isExponent) -> NumberLiteral {
        auto number = NumberLiteralValue{};
        number.radix = radix;

        auto optCp = input.peek();
        auto extendInto = [&](Rope& into) {
            auto it = input.current();
            while (optCp.map(isDigit)) {
                input.extend();
                optCp = input.peek();
                if (optCp.map(isIgnored)) {
                    into += View{it, input.current()};
                    optCp = extendWhile(isIgnored);
                    it = input.current();
                }
            }
            into += View{it, input.current()};
        };
        auto startZero = optCp.map(isZero);
        if (startZero) optCp = extendWhile(isZeroOrIgnored);
        extendInto(number.integerPart);
        if (optCp.map(isDot)) {
            input.extend();
            optCp = input.peek();
            extendInto(number.fractionalPart);
        }
        if (optCp.map(isExponent)) {
            input.extend();
            optCp = input.peek();
            if (optCp.map(isPlus)) {
                number.exponentSign = Sign::positive;
                input.extend();
                optCp = input.peek();
            }
            else if (optCp.map(isMinus)) {
                number.exponentSign = Sign::negative;
                input.extend();
                optCp = input.peek();
            }
            auto expZero = optCp.map(isZero);
            if (expZero) optCp = extendWhile(isZeroOrIgnored);

            extendInto(number.exponentPart);
            if (!expZero && number.exponentPart.isEmpty()) {
                number.radix = Radix::invalid; // 1e- is invalid exponent
            }
        }
        if (!startZero && number.integerPart.isEmpty() && number.fractionalPart.isEmpty()) {
            number.radix = Radix::invalid; // 0x or 0x. is not valid
        }
        return {number, input.range()};
    };

    if (chr == '0') {
        auto optNext = input.peek<1>();
        if (optNext) {
            switch (optNext.value().v) {
            case 'x':
            case 'X': input.extend(2); return withRadix(Radix::hex, isHexDigit, isP);
            case 'o':
            case 'O': input.extend(2); return withRadix(Radix::octal, isOctalDigit, isP);
            case 'b':
            case 'B': input.extend(2); return withRadix(Radix::binary, isBinaryDigit, isP);
            }
        }
    }
    return withRadix(Radix::decimal, isDecimalDigit, isE);
}

} // namespace scanner
