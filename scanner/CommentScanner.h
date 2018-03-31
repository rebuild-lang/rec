#pragma once
#include "FileInput.h"
#include "Token.h"

namespace scanner {

struct CommentScanner {
    static auto scan(FileInput& input, Column tabStops) -> Token {
        size_t length = 1;
        auto makeToken = [&]() -> Token { return {input.range(), CommentLiteral{}}; };
        auto scanLine = [&] {
            do {
                input.extend(tabStops);
                if (!input.hasMorePeek()) return makeToken();
                auto optCp = input.peek();
                if (!optCp) return makeToken(); // force faulty code point to be visible
                auto chr = optCp.value();
                if (chr.isLineSeparator()) return makeToken();
            } while (true);
        };
        auto scanBlock = [&] {
            input.extend();
            auto commentMarker = input.view();
            auto count = commentMarker.byteCount();
            while (input.hasMoreBytes(count)) {
                if (input.currentView(count).isContentEqual(commentMarker)) {
                    input.extend(length);
                    return makeToken();
                }
                auto optCp = input.peek();
                if (!optCp) return makeToken(); // force faulty code point to be visible
                auto chr = optCp.value();
                input.extend(tabStops);
                if (chr.isLineSeparator()) input.nextLine();
            }
            input.finish();
            return makeToken();
        };

        do {
            input.extend(); // no whitespace allowed
            length++;
            if (!input.hasMorePeek()) return makeToken();
            auto optCp = input.peek();
            if (!optCp) return makeToken(); // force faulty code point error to be visible
            auto chr = optCp.value();
            if (chr.v == '\t' || chr.isWhiteSpace()) return scanLine();
            if (chr.v == '#') return scanBlock();
            if (chr.isLineSeparator()) return makeToken();
        } while (true);
    }
};

} // namespace scanner
