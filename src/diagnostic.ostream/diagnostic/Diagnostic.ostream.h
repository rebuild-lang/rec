#include "diagnostic/Diagnostic.h"

#include "meta/Variant.ostream.h"
#include "strings/String.ostream.h"
#include "strings/View.ostream.h"
#include "strings/utf8Decode.h"

#include <algorithm>
#include <ostream>

namespace diagnostic {

inline auto extractUtf8Markers(const strings::View& line, const std::string& byteMarkers) -> std::string {
    auto utf8Markers = std::string(byteMarkers.size(), ' ');
    auto i = 0u;
    for (auto& cp : utf8Decode(line)) {
        cp.visit(
            [&](const strings::DecodedCodePoint& c) {
                auto vi = c.input.begin() - line.begin();
                utf8Markers[i] = byteMarkers[vi];
                i++;
            },
            [](const strings::DecodedError&) {});
    }
    if (utf8Markers.begin() + i < utf8Markers.end()) utf8Markers.erase(utf8Markers.begin() + i, utf8Markers.end());
    return utf8Markers;
}

inline auto operator<<(std::ostream& out, const SourceCodeBlock& code) -> std::ostream& {
    if (!code.fileName.isEmpty()) {
        out << "--> " << code.fileName << ":" << code.sourceLine.v << '\n';
    }
    auto codeLines = std::vector<strings::View>{};
    auto b = code.code.begin();
    for (auto chr : strings::utf8Decode(code.code)) {
        chr.visitSome([&](strings::DecodedCodePoint& dcp) {
            if (dcp.cp.v == '\n') {
                codeLines.emplace_back(b, dcp.input.begin());
                b = dcp.input.end();
            }
        });
    }
    if (b != code.code.end()) codeLines.emplace_back(b, code.code.end());

    auto markers = std::string(code.code.byteCount().v, ' ');
    for (auto& h : code.highlights) {
        if (h.holds<Marker>()) {
            auto& m = h.get<Marker>();
            for (auto i = 0; i < m.span.length; i++) markers[i + m.span.start] = '~';
        }
    }

    auto num = code.sourceLine.v;
    auto numWidth = std::to_string(num + codeLines.size() - 1).size();
    for (auto& line : codeLines) {
        out << std::setw(numWidth) << std::right << num << " |" << line << '\n';
        // TODO(arBmind): perform highlighting if terminal supports it
        auto byteMarkers = std::string(
            markers.begin() + (line.begin() - code.code.begin()), markers.begin() + (line.end() - code.code.begin()));
        auto markerLine = extractUtf8Markers(line, byteMarkers);
        markerLine.erase(std::find(markerLine.rbegin(), markerLine.rend(), '~').base(), markerLine.end());
        if (!markerLine.empty()) {
            out << std::string(numWidth, ' ') << " |" << markerLine << '\n';
        }
        num++;
    }
    return out;
}

inline auto operator<<(std::ostream& out, const CodeBlock& code) -> std::ostream& {
    return out << code.code; //
}

inline auto operator<<(std::ostream& out, const Paragraph& h) -> std::ostream& {
    return out << h.text; // TODO(arBmind): add formatting
}

inline auto operator<<(std::ostream& out, const Headline& h) -> std::ostream& {
    return out << h.text << '\n' << std::string(h.text.byteCount().v, '-');
}

inline auto operator<<(std::ostream& out, const Section& s) -> std::ostream& { //
    return s.visit([&](auto& e) -> std::ostream& { return out << e; });
}

inline auto operator<<(std::ostream& out, const Document& d) -> std::ostream& { //
    for (auto& s : d) {
        out << '\n' << s << '\n';
    }
    return out;
}

inline auto operator<<(std::ostream& out, const Explanation& e) -> std::ostream& { //
    if (!e.title.isEmpty()) out << e.title << "\n";
    return out << e.details;
}

inline auto operator<<(std::ostream& out, const Suggestion& e) -> std::ostream& { //
    if (!e.title.isEmpty()) out << e.title << "\n" << std::string(e.title.byteCount().v, '-') << "\n";
    return out << e.details;
}

inline auto operator<<(std::ostream& out, const Part& part) -> std::ostream& { //
    return part.visit([&](auto& e) -> std::ostream& { return out << e; });
}

inline auto operator<<(std::ostream& out, const Parts& parts) -> std::ostream& { //
    if (parts.empty()) return out << "<missing diagnostics parts>\n";
    if (!parts.front().holds<Explanation>() || parts.front().get<Explanation>().title.isEmpty()) {
        out << "<missing title>\n";
    }
    for (auto& p : parts) {
        out << p << "\n";
    }
    return out;
}

inline auto operator<<(std::ostream& out, const Diagnostic& d) -> std::ostream& { //
    return out << ">>> " << d.code.clazzId << '[' << d.code.number << "]: " << d.parts;
}

} // namespace diagnostic
