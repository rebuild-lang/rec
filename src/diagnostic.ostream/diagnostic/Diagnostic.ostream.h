#include "diagnostic/Diagnostic.h"

#include "meta/Variant.ostream.h"
#include "strings/String.ostream.h"

#include <ostream>

namespace diagnostic {

inline auto operator<<(std::ostream& out, const SourceCodeBlock& code) -> std::ostream& {
    if (code.fileName.isEmpty()) {
        out << "input line: " << code.sourceLine.v << '\n';
    }
    else {
        out << "in \"" << code.fileName << "\" on line: " << code.sourceLine.v << '\n';
    }
    out << code.code; // TODO(arBmind): add formatting
    if (!code.highlights.empty()) {
        auto markerLine = std::string(code.code.byteCount().v, ' ');
        for (auto& h : code.highlights) {
            if (h.holds<Marker>()) {
                auto& m = h.get<Marker>();
                for (auto i = 0; i < m.span.length; i++) markerLine[i + m.span.start] = '~';
            }
        }
        out << '\n' << markerLine;
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
    // if (!e.title.isEmpty()) out << e.title << "\n" << std::string(e.title.byteCount().v, '-') << "\n";
    // return out << e.details;
    return out;
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
