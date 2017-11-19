#pragma once
#include "meta/optional.h"

#include <cinttypes>

namespace strings {

/// naively strong typed counter
struct count_t {
    using this_t = count_t;
    uint32_t v;

    constexpr bool operator==(this_t o) const noexcept { return v == o.v; }
    constexpr bool operator!=(this_t o) const noexcept { return v != o.v; }

    constexpr auto operator+(this_t c) const noexcept { return this_t{v + c.v}; }
};

/// naively strong typed decimal value
struct decimal_t {
    using this_t = decimal_t;
    uint8_t v; // valid 0..9

    constexpr decimal_t() noexcept
        : v(0xFF) {} // invalid
    constexpr explicit decimal_t(uint8_t n) noexcept
        : v(n) {}

    constexpr decimal_t(const this_t &) noexcept = default;
    constexpr this_t &operator=(const this_t &) noexcept = default;
    constexpr decimal_t(this_t &&) noexcept = default;
    constexpr this_t &operator=(this_t &&) noexcept = default;

    // these enable value packed optional
    constexpr bool operator==(this_t o) const noexcept { return v == o.v; }
    constexpr bool operator!=(this_t o) const noexcept { return v != o.v; }
};
using optional_decimal_t = meta::optional<meta::packed<decimal_t>>;

/// naively strong typed unicode code point
struct code_point_t {
    using this_t = code_point_t;
    uint32_t v;

    constexpr code_point_t() noexcept
        : v(0xFFFFFFFEu) {} // invalid
    constexpr explicit code_point_t(uint32_t n) noexcept
        : v(n) {}

    constexpr code_point_t(const this_t &) noexcept = default;
    constexpr this_t &operator=(const this_t &) noexcept = default;
    constexpr code_point_t(this_t &&) noexcept = default;
    constexpr this_t &operator=(this_t &&) noexcept = default;

    // these enable value packed optional
    constexpr bool operator==(this_t o) const noexcept { return v == o.v; }
    constexpr bool operator!=(this_t o) const noexcept { return v != o.v; }

    // allow easy comparison
    constexpr bool operator==(const uint32_t o) const { return v == o; }
    constexpr bool operator!=(const uint32_t o) const { return v != o; }
    constexpr bool operator<(const uint32_t o) const { return v < o; }
    constexpr bool operator>(const uint32_t o) const { return v > o; }

    constexpr bool is_control() const {
        // see https://www.compart.com/en/unicode/category/Cc
        switch (v) {
            /* clang-format off */
        case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
        case 0x8: case 0x9: case 0xA: case 0xB: case 0xC: case 0xD: case 0xE: case 0xF:
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0x7F:
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
            /* clang-format on */
            return true;
        }
        return false;
    }

    constexpr bool is_line_separator() const {
        switch (v) {
        case 0x2028: // Line Separator (see https://www.compart.com/en/unicode/category/Zl)
        // see Paragraph Separator https://www.compart.com/en/unicode/bidiclass/B
        case 0xA: // Line Feed
        case 0xD: // Carriage Return
        case 0x1C: // File Separator
        case 0x1D: // Group Separator
        case 0x1E: // Record Separator
        case 0x85: // Next Line
        case 0x2029: // Paragraph Separator
            return true;
        }
        return false;
    }

    constexpr bool is_white_space() const {
        // see https://www.compart.com/en/unicode/bidiclass/WS
        switch (v) {
        case 0x000C: // Form Feed
        case 0x0020: // Space
        case 0x00A0: // No-break space
        case 0x1680: // Ogham Space Mark
        case 0x2000: // En Quad
        case 0x2001: // Em Quad
        case 0x2002: // En Space
        case 0x2003: // Em Space
        case 0x2004: // Three-per-em Space
        case 0x2005: // Four-per-em Space
        case 0x2006: // Six-per-em Space
        case 0x2007: // Figure Space
        case 0x2008: // Punctuation Space
        case 0x2009: // Thin Space
        case 0x200A: // Hair Space
        case 0x202F: // Narrow No-break Space
        case 0x205F: // Medium Mathematical Space
        case 0x3000: // <>
            return true;
        }
        return false;
    }

    constexpr bool is_decimal_number() const { return decimal_number(); }

    constexpr auto decimal_number() const -> optional_decimal_t {
        // see https://www.compart.com/en/unicode/category/Nd
        /* clang-format off */
        switch (v) {
        case 0x0030: case 0x0031: case 0x0032: case 0x0033: case 0x0034: case 0x0035: case 0x0036: case 0x0037: case 0x0038: case 0x0039:
            return decimal_t{static_cast<uint8_t>(v - 0x0030)};
        case 0x0660: case 0x0661: case 0x0662: case 0x0663: case 0x0664: case 0x0665: case 0x0666: case 0x0667: case 0x0668: case 0x0669:
            return decimal_t{static_cast<uint8_t>(v - 0x0660)};
        case 0x06F0: case 0x06F1: case 0x06F2: case 0x06F3: case 0x06F4: case 0x06F5: case 0x06F6: case 0x06F7: case 0x06F8: case 0x06F9:
            return decimal_t{static_cast<uint8_t>(v - 0x06F0)};
        case 0x07C0: case 0x07C1: case 0x07C2: case 0x07C3: case 0x07C4: case 0x07C5: case 0x07C6: case 0x07C7: case 0x07C8: case 0x07C9:
            return decimal_t{static_cast<uint8_t>(v - 0x07C0)};
        case 0x0966: case 0x0967: case 0x0968: case 0x0969: case 0x096A: case 0x096B: case 0x096C: case 0x096D: case 0x096E: case 0x096F:
            return decimal_t{static_cast<uint8_t>(v - 0x0966)};
        case 0x09E6: case 0x09E7: case 0x09E8: case 0x09E9: case 0x09EA: case 0x09EB: case 0x09EC: case 0x09ED: case 0x09EE: case 0x09EF:
            return decimal_t{static_cast<uint8_t>(v - 0x09E6)};
        case 0x0A66: case 0x0A67: case 0x0A68: case 0x0A69: case 0x0A6A: case 0x0A6B: case 0x0A6C: case 0x0A6D: case 0x0A6E: case 0x0A6F:
            return decimal_t{static_cast<uint8_t>(v - 0x0A66)};
        case 0x0AE6: case 0x0AE7: case 0x0AE8: case 0x0AE9: case 0x0AEA: case 0x0AEB: case 0x0AEC: case 0x0AED: case 0x0AEE: case 0x0AEF:
            return decimal_t{static_cast<uint8_t>(v - 0x0AE6)};
        case 0x0B66: case 0x0B67: case 0x0B68: case 0x0B69: case 0x0B6A: case 0x0B6B: case 0x0B6C: case 0x0B6D: case 0x0B6E: case 0x0B6F:
            return decimal_t{static_cast<uint8_t>(v - 0x0B66)};
        case 0x0BE6: case 0x0BE7: case 0x0BE8: case 0x0BE9: case 0x0BEA: case 0x0BEB: case 0x0BEC: case 0x0BED: case 0x0BEE: case 0x0BEF:
            return decimal_t{static_cast<uint8_t>(v - 0x0BE6)};
        case 0x0C66: case 0x0C67: case 0x0C68: case 0x0C69: case 0x0C6A: case 0x0C6B: case 0x0C6C: case 0x0C6D: case 0x0C6E: case 0x0C6F:
            return decimal_t{static_cast<uint8_t>(v - 0x0C66)};
        case 0x0CE6: case 0x0CE7: case 0x0CE8: case 0x0CE9: case 0x0CEA: case 0x0CEB: case 0x0CEC: case 0x0CED: case 0x0CEE: case 0x0CEF:
            return decimal_t{static_cast<uint8_t>(v - 0x0CE6)};
        case 0x0D66: case 0x0D67: case 0x0D68: case 0x0D69: case 0x0D6A: case 0x0D6B: case 0x0D6C: case 0x0D6D: case 0x0D6E: case 0x0D6F:
            return decimal_t{static_cast<uint8_t>(v - 0x0D66)};
        case 0x0DE6: case 0x0DE7: case 0x0DE8: case 0x0DE9: case 0x0DEA: case 0x0DEB: case 0x0DEC: case 0x0DED: case 0x0DEE: case 0x0DEF:
            return decimal_t{static_cast<uint8_t>(v - 0x0DE6)};
        case 0x0E50: case 0x0E51: case 0x0E52: case 0x0E53: case 0x0E54: case 0x0E55: case 0x0E56: case 0x0E57: case 0x0E58: case 0x0E59:
            return decimal_t{static_cast<uint8_t>(v - 0x0E50)};
        case 0x0ED0: case 0x0ED1: case 0x0ED2: case 0x0ED3: case 0x0ED4: case 0x0ED5: case 0x0ED6: case 0x0ED7: case 0x0ED8: case 0x0ED9:
            return decimal_t{static_cast<uint8_t>(v - 0x0ED0)};
        case 0x0F20: case 0x0F21: case 0x0F22: case 0x0F23: case 0x0F24: case 0x0F25: case 0x0F26: case 0x0F27: case 0x0F28: case 0x0F29:
            return decimal_t{static_cast<uint8_t>(v - 0x0F20)};
        case 0x1040: case 0x1041: case 0x1042: case 0x1043: case 0x1044: case 0x1045: case 0x1046: case 0x1047: case 0x1048: case 0x1049:
            return decimal_t{static_cast<uint8_t>(v - 0x1040)};
        case 0x1090: case 0x1091: case 0x1092: case 0x1093: case 0x1094: case 0x1095: case 0x1096: case 0x1097: case 0x1098: case 0x1099:
            return decimal_t{static_cast<uint8_t>(v - 0x1090)};
        case 0x17E0: case 0x17E1: case 0x17E2: case 0x17E3: case 0x17E4: case 0x17E5: case 0x17E6: case 0x17E7: case 0x17E8: case 0x17E9:
            return decimal_t{static_cast<uint8_t>(v - 0x17E0)};
        case 0x1810: case 0x1811: case 0x1812: case 0x1813: case 0x1814: case 0x1815: case 0x1816: case 0x1817: case 0x1818: case 0x1819:
            return decimal_t{static_cast<uint8_t>(v - 0x1810)};
        case 0x1946: case 0x1947: case 0x1948: case 0x1949: case 0x194A: case 0x194B: case 0x194C: case 0x194D: case 0x194E: case 0x194F:
            return decimal_t{static_cast<uint8_t>(v - 0x1946)};
        case 0x19D0: case 0x19D1: case 0x19D2: case 0x19D3: case 0x19D4: case 0x19D5: case 0x19D6: case 0x19D7: case 0x19D8: case 0x19D9:
            return decimal_t{static_cast<uint8_t>(v - 0x19D0)};
        case 0x1A80: case 0x1A81: case 0x1A82: case 0x1A83: case 0x1A84: case 0x1A85: case 0x1A86: case 0x1A87: case 0x1A88: case 0x1A89:
            return decimal_t{static_cast<uint8_t>(v - 0x1A80)};
        case 0x1A90: case 0x1A91: case 0x1A92: case 0x1A93: case 0x1A94: case 0x1A95: case 0x1A96: case 0x1A97: case 0x1A98: case 0x1A99:
            return decimal_t{static_cast<uint8_t>(v - 0x1A90)};
        case 0x1B50: case 0x1B51: case 0x1B52: case 0x1B53: case 0x1B54: case 0x1B55: case 0x1B56: case 0x1B57: case 0x1B58: case 0x1B59:
            return decimal_t{static_cast<uint8_t>(v - 0x1B50)};
        case 0x1BB0: case 0x1BB1: case 0x1BB2: case 0x1BB3: case 0x1BB4: case 0x1BB5: case 0x1BB6: case 0x1BB7: case 0x1BB8: case 0x1BB9:
            return decimal_t{static_cast<uint8_t>(v - 0x1BB0)};
        case 0x1C40: case 0x1C41: case 0x1C42: case 0x1C43: case 0x1C44: case 0x1C45: case 0x1C46: case 0x1C47: case 0x1C48: case 0x1C49:
            return decimal_t{static_cast<uint8_t>(v - 0x1C40)};
        case 0x1C50: case 0x1C51: case 0x1C52: case 0x1C53: case 0x1C54: case 0x1C55: case 0x1C56: case 0x1C57: case 0x1C58: case 0x1C59:
            return decimal_t{static_cast<uint8_t>(v - 0x1C50)};
        case 0xA620: case 0xA621: case 0xA622: case 0xA623: case 0xA624: case 0xA625: case 0xA626: case 0xA627: case 0xA628: case 0xA629:
            return decimal_t{static_cast<uint8_t>(v - 0xA620)};
        case 0xA8D0: case 0xA8D1: case 0xA8D2: case 0xA8D3: case 0xA8D4: case 0xA8D5: case 0xA8D6: case 0xA8D7: case 0xA8D8: case 0xA8D9:
            return decimal_t{static_cast<uint8_t>(v - 0xA8D0)};
        case 0xA900: case 0xA901: case 0xA902: case 0xA903: case 0xA904: case 0xA905: case 0xA906: case 0xA907: case 0xA908: case 0xA909:
            return decimal_t{static_cast<uint8_t>(v - 0xA900)};
        case 0xA9D0: case 0xA9D1: case 0xA9D2: case 0xA9D3: case 0xA9D4: case 0xA9D5: case 0xA9D6: case 0xA9D7: case 0xA9D8: case 0xA9D9:
            return decimal_t{static_cast<uint8_t>(v - 0xA9D0)};
        case 0xA9F0: case 0xA9F1: case 0xA9F2: case 0xA9F3: case 0xA9F4: case 0xA9F5: case 0xA9F6: case 0xA9F7: case 0xA9F8: case 0xA9F9:
            return decimal_t{static_cast<uint8_t>(v - 0xA9F0)};
        case 0xAA50: case 0xAA51: case 0xAA52: case 0xAA53: case 0xAA54: case 0xAA55: case 0xAA56: case 0xAA57: case 0xAA58: case 0xAA59:
            return decimal_t{static_cast<uint8_t>(v - 0xAA50)};
        case 0xABF0: case 0xABF1: case 0xABF2: case 0xABF3: case 0xABF4: case 0xABF5: case 0xABF6: case 0xABF7: case 0xABF8: case 0xABF9:
            return decimal_t{static_cast<uint8_t>(v - 0xABF0)};
        case 0xFF10: case 0xFF11: case 0xFF12: case 0xFF13: case 0xFF14: case 0xFF15: case 0xFF16: case 0xFF17: case 0xFF18: case 0xFF19:
            return decimal_t{static_cast<uint8_t>(v - 0xFF10)};
        case 0x104A0: case 0x104A1: case 0x104A2: case 0x104A3: case 0x104A4: case 0x104A5: case 0x104A6: case 0x104A7: case 0x104A8: case 0x104A9:
            return decimal_t{static_cast<uint8_t>(v - 0x104A0)};
        case 0x11066: case 0x11067: case 0x11068: case 0x11069: case 0x1106A: case 0x1106B: case 0x1106C: case 0x1106D: case 0x1106E: case 0x1106F:
            return decimal_t{static_cast<uint8_t>(v - 0x11066)};
        case 0x110F0: case 0x110F1: case 0x110F2: case 0x110F3: case 0x110F4: case 0x110F5: case 0x110F6: case 0x110F7: case 0x110F8: case 0x110F9:
            return decimal_t{static_cast<uint8_t>(v - 0x110F0)};
        case 0x11136: case 0x11137: case 0x11138: case 0x11139: case 0x1113A: case 0x1113B: case 0x1113C: case 0x1113D: case 0x1113E: case 0x1113F:
            return decimal_t{static_cast<uint8_t>(v - 0x11136)};
        case 0x111D0: case 0x111D1: case 0x111D2: case 0x111D3: case 0x111D4: case 0x111D5: case 0x111D6: case 0x111D7: case 0x111D8: case 0x111D9:
            return decimal_t{static_cast<uint8_t>(v - 0x111D0)};
        case 0x112F0: case 0x112F1: case 0x112F2: case 0x112F3: case 0x112F4: case 0x112F5: case 0x112F6: case 0x112F7: case 0x112F8: case 0x112F9:
            return decimal_t{static_cast<uint8_t>(v - 0x112F0)};
        case 0x11450: case 0x11451: case 0x11452: case 0x11453: case 0x11454: case 0x11455: case 0x11456: case 0x11457: case 0x11458: case 0x11459:
            return decimal_t{static_cast<uint8_t>(v - 0x11450)};
        case 0x114D0: case 0x114D1: case 0x114D2: case 0x114D3: case 0x114D4: case 0x114D5: case 0x114D6: case 0x114D7: case 0x114D8: case 0x114D9:
            return decimal_t{static_cast<uint8_t>(v - 0x114D0)};
        case 0x11650: case 0x11651: case 0x11652: case 0x11653: case 0x11654: case 0x11655: case 0x11656: case 0x11657: case 0x11658: case 0x11659:
            return decimal_t{static_cast<uint8_t>(v - 0x11650)};
        case 0x116C0: case 0x116C1: case 0x116C2: case 0x116C3: case 0x116C4: case 0x116C5: case 0x116C6: case 0x116C7: case 0x116C8: case 0x116C9:
            return decimal_t{static_cast<uint8_t>(v - 0x116C0)};
        case 0x11730: case 0x11731: case 0x11732: case 0x11733: case 0x11734: case 0x11735: case 0x11736: case 0x11737: case 0x11738: case 0x11739:
            return decimal_t{static_cast<uint8_t>(v - 0x11730)};
        case 0x118E0: case 0x118E1: case 0x118E2: case 0x118E3: case 0x118E4: case 0x118E5: case 0x118E6: case 0x118E7: case 0x118E8: case 0x118E9:
            return decimal_t{static_cast<uint8_t>(v - 0x118E0)};
        case 0x11C50: case 0x11C51: case 0x11C52: case 0x11C53: case 0x11C54: case 0x11C55: case 0x11C56: case 0x11C57: case 0x11C58: case 0x11C59:
            return decimal_t{static_cast<uint8_t>(v - 0x11C50)};
        case 0x16A60: case 0x16A61: case 0x16A62: case 0x16A63: case 0x16A64: case 0x16A65: case 0x16A66: case 0x16A67: case 0x16A68: case 0x16A69:
            return decimal_t{static_cast<uint8_t>(v - 0x16A60)};
        case 0x16B50: case 0x16B51: case 0x16B52: case 0x16B53: case 0x16B54: case 0x16B55: case 0x16B56: case 0x16B57: case 0x16B58: case 0x16B59:
            return decimal_t{static_cast<uint8_t>(v - 0x16B50)};
        case 0x1D7CE: case 0x1D7CF: case 0x1D7D0: case 0x1D7D1: case 0x1D7D2: case 0x1D7D3: case 0x1D7D4: case 0x1D7D5: case 0x1D7D6: case 0x1D7D7:
            return decimal_t{static_cast<uint8_t>(v - 0x1D7CE)};
        case 0x1D7D8: case 0x1D7D9: case 0x1D7DA: case 0x1D7DB: case 0x1D7DC: case 0x1D7DD: case 0x1D7DE: case 0x1D7DF: case 0x1D7E0: case 0x1D7E1:
            return decimal_t{static_cast<uint8_t>(v - 0x1D7D8)};
        case 0x1D7E2: case 0x1D7E3: case 0x1D7E4: case 0x1D7E5: case 0x1D7E6: case 0x1D7E7: case 0x1D7E8: case 0x1D7E9: case 0x1D7EA: case 0x1D7EB:
            return decimal_t{static_cast<uint8_t>(v - 0x1D7E0)};
        case 0x1D7EC: case 0x1D7ED: case 0x1D7EE: case 0x1D7EF: case 0x1D7F0: case 0x1D7F1: case 0x1D7F2: case 0x1D7F3: case 0x1D7F4: case 0x1D7F5:
            return decimal_t{static_cast<uint8_t>(v - 0x1D7EE)};
        case 0x1D7F6: case 0x1D7F7: case 0x1D7F8: case 0x1D7F9: case 0x1D7FA: case 0x1D7FB: case 0x1D7FC: case 0x1D7FD: case 0x1D7FE: case 0x1D7FF:
            return decimal_t{static_cast<uint8_t>(v - 0x1D7F6)};
        case 0x1E950: case 0x1E951: case 0x1E952: case 0x1E953: case 0x1E954: case 0x1E955: case 0x1E956: case 0x1E957: case 0x1E958: case 0x1E959:
            return decimal_t{static_cast<uint8_t>(v - 0x1E950)};
        }
        /* clang-format on */
        return {};
    }

    constexpr count_t utf8_byte_count() const {
        // see https://en.wikipedia.org/wiki/UTF-8
        if (v < 0x80) return {1};
        if (v < 0x800) return {2};
        if (v < 0x10000) return {3};
        if (v < 0x110000) return {4};
        return {0};
    }

    template<class Out>
    void utf8_encode(Out &out) const {
        // see https://en.wikipedia.org/wiki/UTF-8
        if (v < 0x80) {
            out.push_back(v & 0x7F);
        }
        else if (v < 0x800) {
            out.push_back(0xC0 | ((v >> 6) & 0x1F));
            out.push_back(0x80 | ((v >> 0) & 0x3F));
        }
        else if (v < 0x10000) {
            out.push_back(0xE0 | ((v >> 12) & 0xF));
            out.push_back(0x80 | ((v >> 6) & 0x3F));
            out.push_back(0x80 | ((v >> 0) & 0x3F));
        }
        else if (v < 0x110000) {
            out.push_back(0xF0 | ((v >> 18) & 0x7));
            out.push_back(0x80 | ((v >> 12) & 0x3F));
            out.push_back(0x80 | ((v >> 6) & 0x3F));
            out.push_back(0x80 | ((v >> 0) & 0x3F));
        }
    }
};
using optional_code_point_t = meta::optional<meta::packed<code_point_t>>;

} // namespace strings
