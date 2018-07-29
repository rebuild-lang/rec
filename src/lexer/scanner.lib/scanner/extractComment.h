#pragma once
#include "scanner/Token.h"

#include "text/FileInput.h"

namespace scanner {

/**
 * precondition: input is on the valid start character
 *
 * options:
 * a) #{Non-Whitespace}*({Whitespace}{Non-Newline}*)?{Newline} => line comment
 * b) (marker=#{Non-Whitespace}*#){Any}*{marker} => block comment
 *
 */
inline auto extractComment(text::FileInput& input, text::Column tabStops) -> CommentLiteral {
    size_t markerExtends = 1;
    auto makeToken = [&]() -> CommentLiteral { return {input.range()}; };
    auto scanLine = [&] {
        while (true) {
            input.extend(tabStops);
            if (!input.hasMore()) return makeToken();
            auto optCp = input.peek();
            if (!optCp) return makeToken(); // force faulty code point to be visible
            auto cp = optCp.value();
            if (cp.isLineSeparator()) return makeToken();
        }
    };
    auto scanBlock = [&] {
        input.extend();
        auto marker = input.view();
        auto markerBytes = marker.byteCount();
        while (input.hasMoreBytes(markerBytes)) {
            if (input.currentView(markerBytes).isContentEqual(marker)) {
                input.extend(markerExtends);
                return makeToken();
            }
            auto optCp = input.peek();
            if (!optCp) return makeToken(); // force faulty code point to be visible
            auto cp = optCp.value();
            input.extend(tabStops);
            if (cp.isLineSeparator()) input.nextLine();
        }
        input.finish();
        return makeToken();
    };

    while (true) {
        input.extend(); // no whitespace allowed
        markerExtends++;
        if (!input.hasMore()) return makeToken();
        auto optCp = input.peek();
        if (!optCp) return makeToken(); // force faulty code point error to be visible
        auto cp = optCp.value();
        if (cp == '\t' || cp.isWhiteSpace()) return scanLine();
        if (cp == '#') return scanBlock();
        if (cp.isLineSeparator()) return makeToken();
    }
}

} // namespace scanner
