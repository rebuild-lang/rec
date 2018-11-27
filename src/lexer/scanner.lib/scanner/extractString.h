#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>

#include <strings/Rope.h>

namespace scanner {

using strings::CodePoint;
using text::CodePointPosition;
using text::DecodedPosition;
using OptDecodedPosition = meta::Optional<DecodedPosition>;
using OptCodePointPosition = meta::Optional<CodePointPosition>;

/** \brief parse a string literal from an enumerator
 * key features:
 * * "" => empty string
 * * """raw""" => raw string
 * * whitespaces before newlines are skipped
 * * backslash escapes are handled
 * * decodeErrors are eaten
 * * errors are tracked
 */
inline auto extractString(CodePointPosition firstCpp, meta::CoEnumerator<DecodedPosition>& decoded) -> StringLiteral {
    auto string = StringLiteralValue{};

    auto end = firstCpp.input.end();
    auto endPosition = firstCpp.position;
    auto updateEnd = [&](auto ip) {
        end = ip.input.end();
        endPosition = ip.position;
    };
    auto inputView = [&, begin = firstCpp.input.begin()] { return View{begin, end}; };

    auto peekCpp = [&]() -> OptCodePointPosition {
        while (true) {
            if (!decoded) return {};
            auto dp = *decoded;
            if (dp.holds<text::CodePointPosition>()) {
                return dp.get<CodePointPosition>();
            }
            if (dp.holds<text::DecodedErrorPosition>()) {
                auto dep = dp.get<text::DecodedErrorPosition>();
                string.errors.push_back({StringError::Kind::InvalidEncoding, dep.input, dep.position});
                updateEnd(dep);
                decoded++;
                continue;
            }
            return {};
        }
    };
    auto nextCpp = [&]() -> OptCodePointPosition {
        updateEnd((*decoded).get<text::CodePointPosition>());
        decoded++;
        return peekCpp();
    };

    auto addDecodeError = [&](text::DecodedErrorPosition& dep) {
        string.errors.push_back({StringError::Kind::InvalidEncoding, dep.input, dep.position});
    };
    auto addEndOfInputError = [&] { string.errors.push_back({StringError::Kind::EndOfInput, View{}, endPosition}); };
    auto addInvalidControl = [&](const CodePointPosition& cpp) {
        string.errors.push_back({StringError::Kind::InvalidControl, cpp.input, cpp.position});
    };

    static auto isDoubleQuote = [](CodePoint cp) { return cp.v == '"'; };
    static auto isBackslash = [](CodePoint cp) { return cp.v == '\\'; };
    static auto isTab = [](CodePoint cp) { return cp.v == '\t'; };

    auto raw = [&] {
        auto spaces = Rope{};

        auto handleQuotes = [&](const CodePointPosition& firstQuoteCpp) {
            auto optSecondQuoteCpp = peekCpp(); // peek after "
            auto mapCp = [&](auto opt, auto pred) {
                return opt.map([&](auto cpp) -> bool { return pred(cpp.codePoint); });
            };
            if (!mapCp(optSecondQuoteCpp, isDoubleQuote)) {
                string.text += firstQuoteCpp.input;
                return true; // "
            }
            auto optThirdQuoteCpp = nextCpp(); // peek after ""
            if (!mapCp(optThirdQuoteCpp, isDoubleQuote)) {
                string.text += firstQuoteCpp.input;
                string.text += optSecondQuoteCpp.value().input;
                return true; // ""
            }
            auto optForthQuoteCpp = nextCpp(); // peek after """
            if (!mapCp(optForthQuoteCpp, isDoubleQuote)) {
                return false; // end of raw string
            }
            auto optFifthQuoteCpp = nextCpp();
            if (!mapCp(optFifthQuoteCpp, isDoubleQuote)) {
                string.text += firstQuoteCpp.input;
                return false; // " + end of raw string
            }
            auto optSixtQuoteCpp = nextCpp();
            if (!mapCp(optSixtQuoteCpp, isDoubleQuote)) {
                string.text += firstQuoteCpp.input;
                string.text += optSecondQuoteCpp.value().input;
                return false; // "" + end of raw string
            }
            decoded++;
            updateEnd(optSixtQuoteCpp.value());
            string.text += firstQuoteCpp.input;
            string.text += optSecondQuoteCpp.value().input;
            string.text += optThirdQuoteCpp.value().input;
            return true; // """
        };

        decoded++;
        while (decoded) {
            auto dp = *decoded;
            decoded++;
            auto next = dp.visit(
                [&](DecodedErrorPosition& dep) {
                    updateEnd(dep);
                    addDecodeError(dep);
                    return true;
                },
                [&](text::NewlinePosition& nlp) {
                    updateEnd(nlp);
                    string.text += nlp.input;
                    spaces = {};
                    return true;
                },
                [&](CodePointPosition& cpp) {
                    updateEnd(cpp);
                    auto cp = cpp.codePoint;
                    if (cp.isWhiteSpace() || isTab(cp)) {
                        spaces += cpp.input;
                        return true;
                    }
                    string.text += spaces;
                    spaces = {};
                    if (isDoubleQuote(cp)) {
                        return handleQuotes(cpp);
                    }
                    string.text += cpp.input;
                    return true;
                });
            if (!next) return;
        }
        addEndOfInputError(); // out of input
    };

    auto handleEscape = [&](CodePointPosition& cpp) {
        auto addEscapeError = [&](StringError::Kind kind) {
            string.errors.push_back({kind, View{cpp.input.begin(), end}, cpp.position});
        };
        auto hexUnicode = [&] {
            auto unicode = CodePoint{0};

            auto optCpp = peekCpp();
            for (int i = 0; i < 6; i++) { // 0x10FFFF
                if (!optCpp) {
                    if (unicode.v != 0) string.text += unicode;
                    return;
                }
                auto cp = optCpp.value().codePoint;
                if (cp.v >= '0' && cp.v <= '9') {
                    unicode.v = (unicode.v << 4) + (cp.v - '0');
                }
                else if (cp.v >= 'a' && cp.v <= 'f') {
                    unicode.v = (unicode.v << 4) + (10 + cp.v - 'a');
                }
                else if (cp.v >= 'A' && cp.v <= 'F') {
                    unicode.v = (unicode.v << 4) + (10 + cp.v - 'A');
                }
                else {
                    break;
                }
                optCpp = nextCpp();
            }
            if (unicode.v > 0 && unicode.v <= 0x10FFFF) {
                string.text += unicode;
            }
            else {
                addEscapeError(StringError::Kind::InvalidHexUnicode);
            }
        };
        auto decimalUnicode = [&] {
            auto unicode = CodePoint{0};

            auto optCpp = peekCpp();
            for (int i = 0; i < 7; i++) { // 0 .. 1114111
                if (!optCpp) {
                    if (unicode.v != 0) string.text += unicode;
                    return;
                }
                auto cp = optCpp.value().codePoint;
                if (cp.v >= '0' && cp.v <= '9') {
                    unicode.v = (unicode.v * 10) + (cp.v - '0');
                }
                else {
                    break;
                }
                optCpp = nextCpp();
            }
            if (unicode.v > 0 && unicode.v <= 0x10FFFF) {
                string.text += unicode;
            }
            else {
                addEscapeError(StringError::Kind::InvalidDecimalUnicode);
            }
        };

        auto optCpp = peekCpp();
        if (!optCpp) return;
        decoded++;
        updateEnd(optCpp.value());

        auto cp = optCpp.value().codePoint;
        switch (cp.v) {
        case '0': string.text += CodePoint{0}; break;
        case '"':
        case '\\': string.text += cp; break;
        case 't': string.text += CodePoint{'\t'}; break;
        case 'r': string.text += CodePoint{'\r'}; break;
        case 'n': string.text += CodePoint{'\n'}; break;
        case 'x': return hexUnicode();
        case 'u': return decimalUnicode();
        default: return addEscapeError(StringError::Kind::InvalidEscape);
        }
    };

    auto regular = [&] {
        auto spaces = Rope{};

        while (decoded) {
            auto dp = *decoded;
            decoded++;
            auto next = dp.visit(
                [&](DecodedErrorPosition& dep) {
                    updateEnd(dep);
                    addDecodeError(dep);
                    return true;
                },
                [&](text::NewlinePosition& nlp) {
                    updateEnd(nlp);
                    spaces = {};
                    return true;
                },
                [&](CodePointPosition& cpp) {
                    updateEnd(cpp);
                    auto cp = cpp.codePoint;
                    if (cp.isWhiteSpace() || isTab(cp)) {
                        spaces += cpp.input;
                        return true;
                    }
                    string.text += spaces;
                    spaces = {};
                    if (isDoubleQuote(cp)) {
                        return false; // done
                    }
                    if (isBackslash(cp)) {
                        handleEscape(cpp);
                        return true;
                    }
                    if (cp.isControl()) {
                        addInvalidControl(cpp);
                        return true;
                    }
                    string.text += cpp.input;
                    return true;
                });
            if (!next) return;
        }
        addEndOfInputError(); // out of input
    };

    auto optCpp = peekCpp(); // look for 2nd double quote
    auto mapCp = [&](auto pred) { return optCpp.map([&](auto cpp) -> bool { return pred(cpp.codePoint); }); };
    if (mapCp(isDoubleQuote)) {
        optCpp = nextCpp(); // look for 3rd double quote
        if (mapCp(isDoubleQuote)) raw(); // three quotes => raw string
        // two quotes => empty string
    }
    else {
        regular();
    }
    return {string, inputView(), firstCpp.position};
}

} // namespace scanner
