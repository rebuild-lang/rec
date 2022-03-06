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

    using DCP = strings::DecodedCodePoint;
    using DE = strings::DecodedError;

    static constexpr auto isDual = [](auto cp) { return cp == '\n' || cp == '\r'; };
    auto position = Position{};
    auto handleDcp = [&in, &position, &config](DCP dcp) -> DecodedPosition {
        auto [input, cp] = dcp;
        if (cp.isLineSeparator()) {
            auto r = NewlinePosition{input, position};
            // ignore '\r\n' and '\n\r' sequence
            if (isDual(cp.v) && in && in->holds<DCP>()) {
                auto dc2 = in->get<DCP>();
                auto cp2 = dc2.cp;
                if (isDual(cp2.v) && cp2 != cp) {
                    ++in;
                    r.input = View{r.input.begin(), dc2.input.end()};
                }
            }
            position.nextLine();
            return r;
        }
        auto r = CodePointPosition{{input, position}, cp};
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
        while (in && in->holds<DCP>()) {
            auto dc2 = in->get<DCP>();
            auto cp2 = dc2.cp;
            if (!cp2.isCombiningMark()) break;
            ++in;
            r.input = View{r.input.begin(), dc2.input.end()};
        }
        position.nextColumn();
        r.endPosition = position;
        return r;
    };
    auto handleDe = [&position](DE de) -> DecodedPosition { return DecodedErrorPosition{de.input, position}; };
    ++in;
    while (in) {
        auto c = in.move();
        ++in;
        if (c.holds<DE>())
            co_yield handleDe(c.get<DE>());
        else
            co_yield handleDcp(c.get<DCP>());
        // MSVC2022 17.1.0 bugs out on this
        // co_yield c.visit(handleDe, handleDcp);
    }
}
} // namespace text
