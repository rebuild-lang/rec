#pragma once
#include "NumberLiteral.h"
#include "Token.h"

#include "text/FileInput.h"

namespace scanner {

using CodePoint = strings::CodePoint;

struct NumberScanner {

    static auto scan(CodePoint chr, text::FileInput& input) -> NumberLiteral {
        if (chr == '0') {
            auto optNext = input.peek<1>();
            if (optNext) {
                switch (optNext.value().v) {
                case 'x':
                case 'X': input.extend(2); return scanWithRadix(input, Radix::hex, isHexDigit, isP);
                case 'o':
                case 'O': input.extend(2); return scanWithRadix(input, Radix::octal, isOctalDigit, isP);
                case 'b':
                case 'B': input.extend(2); return scanWithRadix(input, Radix::binary, isBinaryDigit, isP);
                }
            }
        }
        return scanWithRadix(input, Radix::decimal, isDecimalDigit, isE);
    }

private:
    template<class Pred>
    static auto extendWhile(text::FileInput& input, Pred pred) -> text::OptCodePoint {
        while (true) {
            input.extend();
            auto chr = input.peek();
            if (chr && pred(chr.value())) continue;
            return chr;
        }
    }

    template<class IsDigit, class IsExponent>
    static auto scanWithRadix(text::FileInput& input, Radix radix, IsDigit isDigit, IsExponent isExponent)
        -> NumberLiteral {
        auto literal = NumberLiteralValue{};
        literal.radix = radix;

        auto chr = input.peek();
        auto intZero = chr.map(isZero);
        if (intZero) {
            chr = extendWhile(input, isZeroOrIgnored);
        }
        auto scanDigitsInto = [&](Rope& into) {
            auto it = input.current();
            while (chr.map(isDigit)) {
                input.extend();
                chr = input.peek();
                if (chr.map(isIgnored)) {
                    into += View{it, input.current()};
                    chr = extendWhile(input, isIgnored);
                    it = input.current();
                }
            }
            into += View{it, input.current()};
        };
        scanDigitsInto(literal.integerPart);
        if (chr.map(isDot)) {
            input.extend();
            chr = input.peek();
            scanDigitsInto(literal.fractionalPart);
        }
        if (chr.map(isExponent)) {
            input.extend();
            chr = input.peek();
            if (chr.map(isPlus)) {
                literal.exponentSign = Sign::positive;
                input.extend();
                chr = input.peek();
            }
            else if (chr.map(isMinus)) {
                literal.exponentSign = Sign::negative;
                input.extend();
                chr = input.peek();
            }
            auto expZero = chr.map(isZero);
            if (expZero) {
                chr = extendWhile(input, isZeroOrIgnored);
            }
            scanDigitsInto(literal.exponentPart);
            if (!expZero && literal.exponentPart.isEmpty()) literal.radix = Radix::invalid;
        }
        if (!intZero && literal.integerPart.isEmpty() && literal.fractionalPart.isEmpty()) {
            literal.radix = Radix::invalid;
        }
        return {literal, input.range()};
    }

    static bool isP(CodePoint _char) { return _char.v == 'p' || _char.v == 'P'; }
    static bool isE(CodePoint _char) { return _char.v == 'e' || _char.v == 'E'; }

    static bool isPlus(CodePoint _char) { return _char.v == '+'; }
    static bool isMinus(CodePoint _char) { return _char.v == '-'; }

    static bool isDot(CodePoint _char) { return _char.v == '.'; }

    static bool isZero(CodePoint _char) { return _char.v == '0'; }
    static bool isDecimalZero(CodePoint _char) {
        return _char.decimalNumber() && [](auto decimal) { return decimal == strings::Decimal{0}; };
    }
    static bool isIgnored(CodePoint _char) { return _char.v == '\''; }
    static bool isZeroOrIgnored(CodePoint _char) { return isZero(_char) || isIgnored(_char); }

    static bool isDecimalDigit(CodePoint _char) { return _char.isDecimalNumber(); }
    static bool isBinaryDigit(CodePoint _char) { return (_char.v >= '0' && _char.v <= '1'); }
    static bool isOctalDigit(CodePoint _char) { return (_char.v >= '0' && _char.v <= '7'); }
    static bool isHexDigit(CodePoint _char) {
        auto chr = _char.v;
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    }
};

} // namespace scanner
