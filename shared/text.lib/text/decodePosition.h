#pragma once
#include <meta/CoEnumerator.h>

#include <strings/Decoded.h>

#include "DecodedPosition.h"

namespace text {

struct Config {
    Column tabStops{}; ///< columns per tabstop
};

inline auto decodePosition( //
    meta::CoEnumerator<strings::Decoded> in,
    Config config) -> meta::CoEnumerator<DecodedPosition> {

    using strings::DecodedCodePoint;

    auto isDual = [](auto cp) { return cp == '\n' || cp == '\r'; };
    Position position;
    ++in;
    while (in) {
        auto c = *in;
        ++in;
        co_yield c.visit(
            [&](DecodedCodePoint dcp) -> DecodedPosition {
                auto cp = dcp.cp;
                if (cp.isLineSeparator()) {
                    auto r = NewlinePosition{dcp.input, position};
                    // ignore '\r\n' and '\n\r' sequence
                    if (isDual(cp.v) && in && in->holds<DecodedCodePoint>()) {
                        auto dc2 = in->get<DecodedCodePoint>();
                        auto cp2 = dc2.cp;
                        if (isDual(cp2.v) && cp2 != cp) {
                            ++in;
                            r.input = View{r.input.begin(), dc2.input.end()};
                        }
                    }
                    position.nextLine();
                    return r;
                }
                auto r = CodePointPosition{dcp.input, position, dcp.cp};
                if (cp.isTab()) {
                    position.nextTabstop(config.tabStops);
                    r.endPosition = position;
                    return r;
                }
                if (cp.isControl() || cp.isSurrogate() || cp.isNonCharacter() || cp.isPrivateUse()) {
                    r.endPosition = position;
                    return r; // keep position
                }
                // ignore all Combiningmarks
                while (in && in->holds<DecodedCodePoint>()) {
                    auto dc2 = in->get<DecodedCodePoint>();
                    auto cp2 = dc2.cp;
                    if (!cp2.isCombiningMark()) break;
                    ++in;
                    r.input = View{r.input.begin(), dc2.input.end()};
                }
                position.nextColumn();
                r.endPosition = position;
                return r;
            },
            [&](strings::DecodedError ie) -> DecodedPosition {
                return DecodedErrorPosition{ie.input, position};
            });
    }
}
} // namespace text
