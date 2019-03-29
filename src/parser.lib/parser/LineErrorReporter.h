#pragma once
#include "Context.h"

#include "nesting/Token.h"

#include "strings/Rope.h"
#include "strings/utf8Decode.h"

#include <sstream>

namespace parser {

template<class Context>
void reportLineErrors(const nesting::BlockLine& line, Context& context) {
    line.forEach([&](auto& t) {
        t.visitSome(
            [&](const nesting::InvalidEncoding& ie) { reportInvalidEncoding(line, ie, context); },
            [&](const nesting::CommentLiteral& cl) { reportTokenWithDecodeErrors(line, cl, context); },
            [&](const nesting::IdentifierLiteral& il) { reportTokenWithDecodeErrors(line, il, context); }
            //[&](const nesting::UnexpectedCharacter& uc) { reportUnexpectedCharacter(line, uc, context); }
        );
    });
}

// extends the view so that it starts after a newline and ends before a newline
// note: it will never expand beyond the current blockLine - as we risk to run before the start of the string
inline auto extractViewLines(const nesting::BlockLine& blockLine, strings::View view) -> strings::View {
    auto blockLineBegin = strings::View::It{};
    auto blockLineEnd = strings::View::It{};
    if (!blockLine.tokens.empty()) {
        blockLineBegin = blockLine.tokens.front().visit([](auto& t) { return t.input.begin(); });
        blockLineEnd = blockLine.tokens.back().visit([](auto& t) { return t.input.end(); });
    }
    if (!blockLine.insignificants.empty()) {
        auto start = blockLine.insignificants.front().visit([](auto& t) { return t.input.begin(); });
        auto end = blockLine.insignificants.back().visit([](auto& t) { return t.input.end(); });
        if (!blockLineBegin || start < blockLineBegin) blockLineBegin = start;
        if (!blockLineEnd || end > blockLineEnd) blockLineEnd = end;
    }

    auto begin = view.begin();
    while (begin > blockLineBegin && begin[-1] != '\r' && begin[-1] != '\n') begin--;
    auto end = view.end();
    while (end < blockLineEnd && end[0] != '\r' && end[0] != '\n') end++;
    return strings::View{begin, end};
}

using ViewMarkers = std::vector<strings::View>;

struct EscapedMarkers {
    strings::String escaped;
    diagnostic::TextSpans markers;
};

inline auto escapeSourceLine(strings::View view, ViewMarkers viewMarkers) -> EscapedMarkers {
    auto output = strings::Rope{};
    auto markers = diagnostic::TextSpans{};
    markers.resize(viewMarkers.size(), diagnostic::TextSpan{-1, -1});
    auto begin = view.begin();
    auto offset = 0;
    auto updateMarkers = [&](strings::View::It p) {
        auto i = 0;
        for (const auto& vm : viewMarkers) {
            auto& m = markers[i];
            if (vm.begin() <= p && m.start == -1) m.start = offset;
            if (vm.end() <= p && m.length == -1) m.length = offset - m.start;
            i++;
        }
    };
    auto hasErrors = false;

    for (auto d : strings::utf8Decode(view)) {
        d.visit(
            [&](strings::DecodedCodePoint dcp) {
                updateMarkers(dcp.input.begin());
                if (dcp.cp.v == '\\') {
                    output += strings::View{begin, dcp.input.end()};
                    output += dcp.cp;
                    begin = dcp.input.end();
                    offset += 1;
                }
                offset += 1;
            },
            [&](strings::DecodedError de) {
                hasErrors = true;
                updateMarkers(de.input.begin());
                output += strings::View{begin, de.input.begin()};
                auto escaped = std::stringstream{};
                escaped << "\\[" << std::hex;
                for (auto x : de.input) escaped << ((int)x & 255);
                escaped << "]" << std::dec;
                auto str = escaped.str();
                output += strings::String{str.data(), str.data() + str.size()};
                begin = de.input.end();
                offset += str.length();
            });
    }
    output += strings::View{begin, view.end()};
    updateMarkers(view.end());

    if (!hasErrors) { // do not escape if not necessary
        auto i = 0;
        for (const auto& vm : viewMarkers) {
            auto& m = markers[i];
            m.start = vm.begin() - view.begin();
            m.length = vm.byteCount().v;
        }
        return EscapedMarkers{to_string(view), std::move(markers)};
    }

    return EscapedMarkers{to_string(output), std::move(markers)};
}

template<class Token, class Context>
void reportDecodeErrors(const nesting::BlockLine& blockLine, const Token& tok, ContextApi<Context>& context) {
    using namespace diagnostic;

    auto line = extractViewLines(blockLine, tok.input);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.insignificants) {
        t.visitSome(
            [&](const nesting::InvalidEncoding& ie) {
                if (ie.input.begin() >= line.begin() && ie.input.end() <= line.end()) {
                    viewMarkers.emplace_back(ie.input);
                    if (&ie != (void*)&tok) const_cast<nesting::InvalidEncoding&>(ie).isTainted = true;
                }
            },
            [&](const nesting::CommentLiteral& cl) {
                if (cl.input.begin() >= line.begin() && cl.input.end() <= line.end()) {
                    for (auto& p : cl.decodeErrors) viewMarkers.emplace_back(p.input);
                    if (&cl != (void*)&tok) const_cast<nesting::CommentLiteral&>(cl).isTainted = true;
                }
            },
            [&](const nesting::IdentifierLiteral& il) {
                if (il.input.begin() >= line.begin() && il.input.end() <= line.end()) {
                    for (auto& p : il.decodeErrors) viewMarkers.emplace_back(p.input);
                    if (&il != (void*)&tok) const_cast<nesting::IdentifierLiteral&>(il).isTainted = true;
                }
            });
    }

    auto [escaped, escapedMarkers] = escapeSourceLine(line, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{
        {Paragraph{(viewMarkers.size() == 1) ? String{"The UTF8-decoder encountered an invalid encoding"}
                                             : String{"The UTF8-decoder encountered multiple invalid encodings"},
                   {}},
         SourceCodeBlock{escaped, highlights, String{}, tok.position.line}}};

    auto expl = Explanation{String("Invalid UTF8 Encoding"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 1}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

template<class... Tags, class Context>
void reportTokenWithDecodeErrors(
    const nesting::BlockLine& blockLine,
    const scanner::details::TagTokenWithDecodeErrors<Tags...>& de,
    ContextApi<Context>& context) {
    if (de.isTainted || de.decodeErrors.empty()) return; // already reported or no errors

    reportDecodeErrors(blockLine, de, context);
}

template<class Context>
void reportInvalidEncoding(
    const nesting::BlockLine& blockLine, const nesting::InvalidEncoding& ie, ContextApi<Context>& context) {
    if (ie.isTainted) return; // already reported

    reportDecodeErrors(blockLine, ie, context);
}

} // namespace parser
