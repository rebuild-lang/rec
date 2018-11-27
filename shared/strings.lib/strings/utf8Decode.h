#pragma once
#include <meta/CoEnumerator.h>

#include "Decoded.h"

namespace strings {

inline auto utf8Decode(View view) -> meta::CoEnumerator<Decoded> {
    auto hasData = [&](size_t bytes = 1) { return view.byteCount().v >= bytes; };
    auto peek = [&]() -> uint32_t { return static_cast<uint32_t>(*view.data()); };
    auto take = [&] { view = view.skipBytes<1>(); };

    while (hasData(1)) {
        auto p = view.begin();
        auto decoded = [&](uint32_t cp) { return DecodedCodePoint{View{p, view.begin()}, CodePoint{cp}}; };
        auto wrong = [&]() { return DecodedError{View{p, view.begin()}}; };
        auto outOfData = [&]() { return DecodedError{View{p, view.end()}}; };
        auto c0 = peek();
        take();
        if ((c0 & 0x80u) != 0x80) {
            co_yield decoded(c0);
            continue;
        }

        if ((c0 & 0xE0u) == 0xC0) {
            if (!hasData(1)) {
                co_yield outOfData();
                continue;
            }
            auto c1 = peek();
            if ((c1 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            co_yield decoded(((c0 & 0x1Fu) << 6u) | ((c1 & 0x3Fu) << 0u));
            continue;
        }

        if ((c0 & 0xF0u) == 0xE0) {
            if (!hasData(2)) {
                co_yield outOfData();
                continue;
            }
            auto c1 = peek();
            if ((c1 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            auto c2 = peek();
            if ((c2 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            co_yield decoded(((c0 & 0x0Fu) << 12u) | ((c1 & 0x3Fu) << 6u) | ((c2 & 0x3Fu) << 0u));
            continue;
        }

        if ((c0 & 0xF8u) == 0xF0) {
            if (!hasData(3)) {
                co_yield outOfData();
                continue;
            }
            auto c1 = peek();
            if ((c1 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            auto c2 = peek();
            if ((c2 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            auto c3 = peek();
            if ((c3 & 0xC0u) != 0x80) {
                co_yield wrong();
                continue;
            }
            take();
            co_yield decoded(
                ((c0 & 0x07u) << 18u) | ((c1 & 0x3Fu) << 12u) | ((c2 & 0x3Fu) << 6u) | ((c3 & 0x3Fu) << 0u));
            continue;
        }

        co_yield wrong();
    }
}

} // namespace strings
