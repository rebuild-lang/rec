#pragma once
#include <scanner/Token.h>

#include <meta/CoEnumerator.h>

namespace scanner {

using text::CodePointPosition;
using text::DecodedPosition;

/**
 * precondition: input is on the valid start character
 *
 * options:
 * a) #{Non-Whitespace}*({Whitespace}{Non-Newline}*)?{Newline} => line comment
 * b) (marker=#{Non-Whitespace}*#){Any}*{marker} => block comment
 *
 */
inline auto extractComment(CodePointPosition firstCpp, meta::CoEnumerator<DecodedPosition>& decoded) -> CommentLiteral {
    using strings::CompareView;
    using text::NewlinePosition;
    auto decodeErrors = DecodedErrorPositions{};

    const auto begin = firstCpp.input.begin();
    auto end = firstCpp.input.end();
    auto updateEnd = [&](auto ip) { end = ip.input.end(); };

    auto makeToken = [&, begin = firstCpp.input.begin()]() -> CommentLiteral {
        return {{View{begin, end}, firstCpp.position}, decodeErrors};
    };
    auto scanLine = [&] {
        while (decoded) {
            auto dp = *decoded;
            decoded++;
            auto next = dp.visit(
                [&](DecodedErrorPosition& dep) {
                    updateEnd(dep);
                    decodeErrors.push_back(dep);
                    return true;
                },
                [&](NewlinePosition&) { return false; },
                [&](CodePointPosition& cpp) {
                    updateEnd(cpp);
                    return true;
                });
            if (!next) break;
        }
        return false;
    };
    auto scanBlock = [&] {
        auto marker = CompareView{begin, end};
        while (decoded) {
            auto dp = *decoded;
            decoded++;
            auto next = dp.visit(
                [&](DecodedErrorPosition& dep) {
                    updateEnd(dep);
                    decodeErrors.push_back(dep);
                    return true;
                },
                [&](NewlinePosition& nlp) {
                    updateEnd(nlp);
                    return true;
                },
                [&](CodePointPosition& cpp) {
                    updateEnd(cpp);
                    auto b = cpp.input.end() - marker.size();
                    return !(
                        cpp.codePoint.v == '#' //
                        && marker.end() < b //
                        && CompareView{marker} == CompareView{b, cpp.input.end()});
                });
            if (!next) break;
        }
        return false;
    };

    while (decoded) {
        auto dp = *decoded;
        auto next = dp.visit(
            [&](DecodedErrorPosition& dep) {
                decoded++;
                updateEnd(dep);
                decodeErrors.push_back(dep);
                return true;
            },
            [&](NewlinePosition&) { return false; },
            [&](CodePointPosition& cpp) {
                decoded++;
                updateEnd(cpp);
                auto cp = cpp.codePoint;
                if (cp == '\t' || cp.isWhiteSpace()) return scanLine();
                if (cp == '#') return scanBlock();
                return true;
            });
        if (!next) break;
    }
    return makeToken();
}

} // namespace scanner
