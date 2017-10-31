#pragma once
#include "file_input.h"
#include "number_literal.h"
#include "token.h"

namespace scanner {

struct number_scanner {

    static inline auto scan(char_t chr, file_input_t &input) -> token {
        if (chr == '0') {
            auto opt_next = input.peek_char<1>();
            if (opt_next) {
                switch (opt_next.value().v) {
                case 'x':
                case 'X': input.extend(2); return scan_radix(input, radix_t::hex, isHexDigit, isP);
                case 'o':
                case 'O': input.extend(2); return scan_radix(input, radix_t::octal, isOctalDigit, isP);
                case 'b':
                case 'B': input.extend(2); return scan_radix(input, radix_t::binary, isBinaryDigit, isP);
                }
            }
        }
        return scan_radix(input, radix_t::decimal, isDecimalDigit, isE);
    }

    template<class Pred>
    static auto extend_while(file_input_t &input, Pred pred) {
        while (true) {
            input.extend();
            auto chr = input.peek_char();
            if (chr && pred(chr.value())) continue;
            return chr;
        }
    }

    template<class IsDigit, class IsExponent>
    static auto scan_radix(file_input_t &input, radix_t radix, IsDigit isDigit, IsExponent isExponent) -> token {
        auto literal = number_literal_t{};
        literal.radix = radix;

        auto chr = input.peek_char();
        auto int_zero = chr.map(isZero);
        if (int_zero) {
            chr = extend_while(input, isZeroOrIgnored);
        }
        auto scan_digits_into = [&](rope_t &into) {
            auto it = input.current();
            while (chr.map(isDigit)) {
                input.extend();
                chr = input.peek_char();
                if (chr.map(isIgnored)) {
                    into += view_t{it, input.current()};
                    chr = extend_while(input, isIgnored);
                    it = input.current();
                }
            }
            into += view_t{it, input.current()};
        };
        scan_digits_into(literal.integer_part);
        if (chr.map(isDot)) {
            input.extend();
            chr = input.peek_char();
            scan_digits_into(literal.fractional_part);
        }
        if (chr.map(isExponent)) {
            input.extend();
            chr = input.peek_char();
            if (chr.map(isPlus)) {
                literal.exponent_sign = sign_t::positive;
                input.extend();
                chr = input.peek_char();
            }
            else if (chr.map(isMinus)) {
                literal.exponent_sign = sign_t::negative;
                input.extend();
                chr = input.peek_char();
            }
            auto exp_zero = chr.map(isZero);
            if (exp_zero) {
                chr = extend_while(input, isZeroOrIgnored);
            }
            scan_digits_into(literal.exponent_part);
            if (!exp_zero && literal.exponent_part.is_empty()) literal.radix = radix_t::invalid;
        }
        if (!int_zero && literal.integer_part.is_empty() && literal.fractional_part.is_empty())
            literal.radix = radix_t::invalid;

        return {input.range(), literal};
    }

    static inline bool isP(char_t _char) { return _char.v == 'p' || _char.v == 'P'; }
    static inline bool isE(char_t _char) { return _char.v == 'e' || _char.v == 'E'; }

    static inline bool isPlus(char_t _char) { return _char.v == '+'; }
    static inline bool isMinus(char_t _char) { return _char.v == '-'; }

    static inline bool isDot(char_t _char) { return _char.v == '.'; }

    static inline bool isZero(char_t _char) { return _char.v == '0'; }
    static inline bool isDecimalZero(char_t _char) { return _char.decimal_number() == 0; }
    static inline bool isIgnored(char_t _char) { return _char.v == '\''; }
    static inline bool isZeroOrIgnored(char_t _char) { return isZero(_char) || isIgnored(_char); }

    static inline bool isDecimalDigit(char_t _char) { return _char.is_decimal_number(); }
    static inline bool isBinaryDigit(char_t _char) { return (_char.v >= '0' && _char.v <= '1'); }
    static inline bool isOctalDigit(char_t _char) { return (_char.v >= '0' && _char.v <= '7'); }
    static inline bool isHexDigit(char_t _char) {
        auto chr = _char.v;
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    }
};

} // namespace scanner
