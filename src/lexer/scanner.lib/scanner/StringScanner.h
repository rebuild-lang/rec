#pragma once
#include "scanner/Token.h"

#include "text/FileInput.h"

#include "strings/Rope.h"

namespace scanner {

using CodePoint = strings::CodePoint;

/**
 * features:
 * * "" => empty string
 * * """raw""" => raw string
 * * whitespaces before newlines are skipped
 */
inline auto extractString(text::FileInput& input) -> StringLiteral {
    auto text = Rope{};
    auto errors = StringErrors{};

    auto endOfInput = [&] { errors.push_back({StringError::Kind::EndOfInput, View{}, input.currentPosition()}); };
    auto invalidCodePoint = [&] {
        auto position = input.currentPosition();
        auto start = input.current();
        input.extend();
        errors.push_back({StringError::Kind::InvalidEncoding, View{start, input.current()}, position});
    };
    auto invalidControl = [&] {
        auto position = input.currentPosition();
        auto start = input.current();
        input.extend();
        errors.push_back({StringError::Kind::InvalidControl, View{start, input.current()}, position});
    };

    auto isDoubleQuote = [](CodePoint cp) { return cp.v == '"'; };
    auto isBackslash = [](CodePoint cp) { return cp.v == '\\'; };
    auto isTab = [](CodePoint cp) { return cp.v == '\t'; };

    auto newLine = [&](CodePoint cp) {
        input.extend();
        // skip 2nd char of newline pair
        if (cp == '\n' || cp == '\r') {
            input.peek().map([&](auto cp2) {
                if (cp2 != cp && (cp2 == '\n' || cp2 == '\r')) input.extend();
            });
        }
        input.nextLine();
    };

    auto raw = [&] {
        input.extend(); // skip 3rd doublequote
        auto start = input.current();
        auto lineStart = start;
        auto spaceStart = start;

        auto reset = [&] { spaceStart = lineStart = start = input.current(); };
        auto nonSpace = [&] { lineStart = spaceStart = input.current(); };

        auto flushToLine = [&]() { text += View{start, lineStart}; };
        auto flushToCurrent = [&](size_t offset = 0) { text += View{start, input.current() - offset}; };
        auto flushToSpace = [&] { text += View{start, spaceStart}; };

        while (true) {
            if (!input.hasMore()) {
                flushToCurrent();
                endOfInput();
                return;
            }
            auto optCp = input.peek();
            if (!optCp) {
                flushToCurrent();
                invalidCodePoint();
                reset();
                continue;
            }
            auto cp = optCp.value();
            if (isDoubleQuote(cp)) {
                input.extend(); // "
                if (!input.peek().map(isDoubleQuote)) {
                    nonSpace();
                    continue; // => "
                }
                input.extend(); // ""
                if (!input.peek().map(isDoubleQuote)) {
                    nonSpace();
                    continue; // => ""
                }
                input.extend(); // """
                if (!input.peek().map(isDoubleQuote)) {
                    flushToLine();
                    return; // string end
                }
                input.extend(); // " """
                if (!input.peek().map(isDoubleQuote)) {
                    flushToCurrent(3);
                    return; // " + string end
                }
                input.extend(); // "" """
                if (!input.peek().map(isDoubleQuote)) {
                    flushToCurrent(3);
                    return; // "" + string end
                }
                input.extend(); // """ """
                flushToCurrent(3); // skip """
                reset();
                continue;
            }
            if (cp.isLineSeparator()) {
                lineStart = input.current();
                if (spaceStart != input.current()) {
                    flushToSpace();
                    start = lineStart;
                }
                newLine(cp);
                spaceStart = input.current();
                continue;
            }
            input.extend();
            if (!cp.isWhiteSpace()) nonSpace();
        }
    };

    auto handleEscape = [&] {
        auto start = input.current();
        auto position = input.currentPosition();

        auto escapeError = [&](StringError::Kind kind) {
            errors.push_back({kind, View{start, input.current()}, position});
        };
        auto hexUnicode = [&] {
            input.extend(); // skip x
            // \x
            // \x1_ \xA_
            // \x12 \xAB
            // \x12345678
            auto unicode = CodePoint{0};
            auto storeUnicode = [&] {
                if (unicode.v == 0)
                    escapeError(StringError::Kind::InvalidHexUnicode);
                else
                    text += unicode;
            };

            for (int i = 0; i < 8; i++) {
                auto optCp = input.peek();
                if (!optCp) {
                    if (unicode.v != 0) storeUnicode();
                    invalidCodePoint();
                    return;
                }
                auto cp = optCp.value();
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
                input.extend();
            }
            storeUnicode();
        };
        auto decimalUnicode = [&] {
            input.extend();
            return escapeError(StringError::Kind::InvalidDecimalUnicode); // TODO(arBmind) decimal unicode character
        };

        input.extend(); // skip escape sign
        auto optCp = input.peek();
        if (!optCp) return invalidCodePoint();

        auto cp = optCp.value();
        switch (cp.v) {
        case '0': text += CodePoint{0}; break;
        case '"':
        case '\\': text += cp; break;
        case 't': text += CodePoint{'\t'}; break;
        case 'r': text += CodePoint{'\r'}; break;
        case 'n': text += CodePoint{'\n'}; break;
        case 'x': return hexUnicode();
        case 'u': return decimalUnicode();
        default: return escapeError(StringError::Kind::InvalidEscape);
        }
        input.extend();
    };

    auto regular = [&] {
        auto start = input.current();
        auto spaceStart = start;

        auto reset = [&] { spaceStart = start = input.current(); };
        auto flushTerminal = [&](auto handler) {
            text += View{start, input.current()};
            handler();
        };
        auto flushToCurrent = [&](auto handler) {
            flushTerminal(handler);
            reset();
        };
        auto flushToSpace = [&](auto handler) {
            text += View{start, spaceStart};
            handler();
            reset();
        };

        while (input.hasMore()) {
            auto optCp = input.peek();
            if (!optCp) {
                flushToCurrent(invalidCodePoint);
                continue;
            }
            auto cp = optCp.value();
            if (isDoubleQuote(cp)) {
                flushToCurrent([&] { input.extend(); }); // skip final doublequote
                return; // done
            }
            if (cp.isLineSeparator()) {
                flushToSpace([=] { newLine(cp); });
                continue;
            }
            if (isBackslash(cp)) {
                flushToCurrent(handleEscape);
                continue;
            }
            if (!isTab(cp) && cp.isControl()) {
                flushToCurrent(invalidControl);
                continue;
            }
            input.extend();
            if (!cp.isWhiteSpace()) spaceStart = input.current();
        }
        flushTerminal(endOfInput); // out of input
    };

    input.extend(); // skip 1st doublequote
    if (input.peek().map(isDoubleQuote)) {
        input.extend(); // skip 2nd doublequote
        if (input.peek().map(isDoubleQuote)) raw(); // three quotes => raw string
        // two quotes => empty string
    }
    else {
        regular();
    }
    return StringLiteral{{std::move(text), std::move(errors)}, input.range()};
}

} // namespace scanner
