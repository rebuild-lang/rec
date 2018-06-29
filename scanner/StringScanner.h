#pragma once
#include "FileInput.h"
#include "Token.h"

#include "strings/Rope.h"

namespace scanner {

struct StringScanner {
    static auto scan(FileInput& input) -> Token {
        input.extend();
        auto text = strings::Rope{};
        auto ip = decltype(input.current()){};
        auto isWhitespace = false;
        auto flush = [&] {
            if (ip != nullptr) {
                if (ip != input.current()) text += View{ip, input.current()};
                ip = {};
                isWhitespace = false;
            }
        };

        while (input.hasMorePeek()) {
            auto optCp = input.peek();
            if (!optCp) break;
            auto chr = optCp.value();
            if (isDoubleQuote(chr)) {
                flush();
                input.extend();
                if (text.isEmpty() && input.peek().map(isDoubleQuote)) {
                    scanUnescaped(input, text);
                }
                break;
            }
            if (chr.isLineSeparator()) {
                if (!isWhitespace) flush();
                ip = {};
                input.extend();
                input.nextLine();
                isWhitespace = false;
                continue;
            }
            if (isBackslash(chr)) {
                flush();
                if (!handleEscape(input, text)) break;
                continue;
            }
            if (!isTab(chr) && chr.isControl()) {
                flush();
                input.extend(); // skip random control chars
                continue;
            }
            if (!isWhitespace && chr.isWhiteSpace()) {
                flush();
                isWhitespace = true;
            }
            else {
                isWhitespace = false;
            }
            if (ip == nullptr) ip = input.current();
            input.extend();
        }
        flush();
        return StringLiteral{{std::move(text)}, input.range()};
    }

private:
    static void scanUnescaped(FileInput& input, strings::Rope& text) {
        input.extend();
        auto ip = input.current();
        auto lineIp = ip;
        auto whitespaceIp = decltype(ip){};
        auto flush = [&](size_t offset = 0) {
            if (ip != input.current() - offset) {
                text += View{ip, input.current() - offset};
                lineIp = {};
                ip = input.current();
            }
        };

        while (input.hasMorePeek()) {
            auto optCp = input.peek();
            if (!optCp) break;
            auto chr = optCp.value();
            if (isDoubleQuote(chr)) {
                whitespaceIp = {};
                input.extend(); // "
                if (!input.peek().map(isDoubleQuote)) {
                    lineIp = {};
                    continue; // normal char
                }
                input.extend(); // ""
                if (!input.peek().map(isDoubleQuote)) {
                    lineIp = {};
                    continue; // normal quotes
                }
                input.extend(); // """
                if (!input.peek().map(isDoubleQuote)) {
                    flush(3);
                    break; // string end
                }
                input.extend(); // " """
                if (!input.peek().map(isDoubleQuote)) {
                    flush(3);
                    break; // " + string end
                }
                input.extend(); // "" """
                if (!input.peek().map(isDoubleQuote)) {
                    flush(3);
                    break; // "" + string end
                }
                input.extend(); // """
                flush(3);
                continue;
            }
            if (chr.isLineSeparator()) {
                if (whitespaceIp != nullptr && whitespaceIp != input.current()) {
                    if (ip != whitespaceIp) text += View{ip, whitespaceIp};
                    whitespaceIp = {};
                    ip = input.current();
                }
                lineIp = input.current();
                input.extend();
                input.nextLine();
                continue;
            }
            if (!chr.isWhiteSpace()) {
                lineIp = {};
                whitespaceIp = {};
            }
            else if (whitespaceIp == nullptr) {
                whitespaceIp = input.current();
            }
            input.extend();
        }
        if (lineIp != nullptr) {
            if (ip != lineIp) text += View{ip, lineIp};
        }
        else if (ip != input.current()) {
            text += View{ip, input.current()};
        }
    }

    static auto handleEscape(FileInput& input, strings::Rope& text) -> bool {
        input.extend();
        auto optCp = input.peek();
        if (!optCp) return false;
        auto chr = optCp.value();
        switch (chr.v) {
        case '\0': return false;
        case 't': text += CodePoint{'\t'}; break;
        case 'r': text += CodePoint{'\r'}; break;
        case 'n': text += CodePoint{'\n'}; break;
        // case 'x': // TODO
        // case 'u': // TODO
        default: text += chr;
        }
        input.extend();
        return true;
    }

    static bool isDoubleQuote(CodePoint cp) { return cp.v == '"'; }
    static bool isBackslash(CodePoint cp) { return cp.v == '\\'; }
    static bool isTab(CodePoint cp) { return cp.v == '\t'; }
};

} // namespace scanner
