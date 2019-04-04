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
            [&](const nesting::NewLineIndentation& nli) { reportNewline(line, nli, context); },
            [&](const nesting::CommentLiteral& cl) { reportTokenWithDecodeErrors(line, cl, context); },
            [&](const nesting::IdentifierLiteral& il) { reportTokenWithDecodeErrors(line, il, context); },
            [&](const nesting::UnexpectedCharacter& uc) { reportUnexpectedCharacter(line, uc, context); });
    });
}

inline auto extractBlockLines(const nesting::BlockLine& blockLine) -> strings::View {
    auto begin = strings::View::It{};
    auto end = strings::View::It{};
    if (!blockLine.tokens.empty()) {
        begin = blockLine.tokens.front().visit([](auto& t) { return t.input.begin(); });
        end = blockLine.tokens.back().visit([](auto& t) { return t.input.end(); });
    }
    if (!blockLine.insignificants.empty()) {
        auto begin2 = blockLine.insignificants.front().visit([](auto& t) { return t.input.begin(); });
        auto end2 = blockLine.insignificants.back().visit([](auto& t) { return t.input.end(); });
        if (!begin || begin2 < begin) begin = begin2;
        if (!end || end2 > end) end = end2;
    }
    return strings::View{begin, end};
}

// extends the view so that it starts after a newline and ends before a newline
// note: it will never expand beyond the current blockLine - as we risk to run before the start of the string
inline auto extractViewLines(const nesting::BlockLine& blockLine, strings::View view) -> strings::View {
    auto all = extractBlockLines(blockLine);
    auto begin = view.begin();
    while (begin > all.begin() && begin[-1] != '\r' && begin[-1] != '\n') begin--;
    auto end = view.end();
    while (end < all.end() && end[0] != '\r' && end[0] != '\n') end++;
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
    auto requiresEscapes = false;
    auto addEscaped = [&](strings::View input) {
        output += strings::View{begin, input.begin()};
        auto escaped = std::stringstream{};
        if (input.size() == 1) {
            switch (input.front()) {
            case 0xA: escaped << "\\n\n"; break;
            case 0xD:
                requiresEscapes = true;
                escaped << R"(\r)";
                break;
            case 0x9:
                requiresEscapes = true;
                escaped << R"(\t)";
                break;
            case 0x0:
                requiresEscapes = true;
                escaped << R"(\0)";
                break;
            default:
                requiresEscapes = true;
                escaped << R"(\[)" << std::hex << ((int)input.front() & 255) << "]" << std::dec;
                break;
            }
        }
        else {
            requiresEscapes = true;
            escaped << R"(\[)" << std::hex;
            for (auto x : input) escaped << ((int)x & 255);
            escaped << "]" << std::dec;
        }
        auto str = escaped.str();
        output += strings::String{str.data(), str.data() + str.size()};
        begin = input.end();
        offset += str.length();
    };

    for (auto d : strings::utf8Decode(view)) {
        d.visit(
            [&](strings::DecodedCodePoint dcp) {
                updateMarkers(dcp.input.begin());
                if (dcp.cp.isCombiningMark() || dcp.cp.isControl() || dcp.cp.isNonCharacter() || dcp.cp.isSurrogate()) {
                    addEscaped(dcp.input);
                    return;
                }
                else if (dcp.cp.v == '\\') {
                    output += strings::View{begin, dcp.input.end()};
                    output += dcp.cp;
                    begin = dcp.input.end();
                    offset += 1;
                }
                offset += 1;
            },
            [&](strings::DecodedError de) {
                updateMarkers(de.input.begin());
                addEscaped(de.input);
            });
    }
    output += strings::View{begin, view.end()};
    updateMarkers(view.end());

    if (!requiresEscapes) { // do not escape if not necessary
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

template<class Context>
void reportDecodeErrorMarkers(
    text::Line line,
    strings::View tokenLines,
    const parser::ViewMarkers& viewMarkers,
    parser::ContextApi<Context>& context) {

    using namespace diagnostic;

    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{
        {Paragraph{(viewMarkers.size() == 1) ? String{"The UTF8-decoder encountered an invalid encoding"}
                                             : String{"The UTF8-decoder encountered multiple invalid encodings"},
                   {}},
         SourceCodeBlock{escapedLines, highlights, String{}, line}}};

    auto expl = Explanation{String("Invalid UTF8 Encoding"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 1}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

inline void collectDecodeErrorMarkers(
    ViewMarkers& viewMarkers, const nesting::BlockLine& blockLine, const strings::View& tokenLines, const void* tok) {
    auto isOnLine = [&](strings::View& input) {
        return input.begin() >= tokenLines.begin() && input.end() <= tokenLines.end();
    };
    for (auto& t : blockLine.insignificants) {
        t.visitSome(
            [&](const nesting::InvalidEncoding& ie) {
                if (ie.isTainted || !ie.input.isPartOf(tokenLines)) return;
                viewMarkers.emplace_back(ie.input);
                if (&ie != tok) const_cast<nesting::InvalidEncoding&>(ie).isTainted = true;
            },
            [&](const nesting::CommentLiteral& cl) {
                if (cl.isTainted || !cl.input.isPartOf(tokenLines)) return;
                for (auto& p : cl.decodeErrors) viewMarkers.emplace_back(p.input);
                if (&cl != tok) const_cast<nesting::CommentLiteral&>(cl).isTainted = true;
            },
            [&](const nesting::IdentifierLiteral& il) {
                if (il.isTainted || !il.input.isPartOf(tokenLines)) return;
                for (auto& p : il.decodeErrors) viewMarkers.emplace_back(p.input);
                if (&il != tok) const_cast<nesting::IdentifierLiteral&>(il).isTainted = true;
            },
            [&](const nesting::NewLineIndentation& nli) {
                if (nli.isTainted || !nli.input.isPartOf(tokenLines)) return;
                for (auto& err : nli.value.errors) {
                    if (!err.holds<scanner::DecodedErrorPosition>()) return;
                }
                for (auto& err : nli.value.errors) {
                    err.visitSome(
                        [&](const scanner::DecodedErrorPosition& dep) { viewMarkers.emplace_back(dep.input); });
                }
                if (&nli != tok) const_cast<nesting::NewLineIndentation&>(nli).isTainted = true;
            });
    }
}

template<class Token, class Context>
void reportDecodeErrors(const nesting::BlockLine& blockLine, const Token& tok, ContextApi<Context>& context) {
    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, tok.input);
    auto viewMarkers = ViewMarkers{};
    collectDecodeErrorMarkers(viewMarkers, blockLine, tokenLines, &tok);
    reportDecodeErrorMarkers(tok.position.line, tokenLines, viewMarkers, context);
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

template<class Context>
void reportNewline(
    const nesting::BlockLine& blockLine, const nesting::NewLineIndentation& nli, ContextApi<Context>& context) {
    if (nli.isTainted || !nli.value.hasErrors()) return; // already reported or no errors

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, nli.input);
    {
        auto viewMarkers = ViewMarkers{};
        for (auto& err : nli.value.errors) {
            err.visitSome([&](const scanner::DecodedErrorPosition& dep) { viewMarkers.emplace_back(dep.input); });
        }
        if (!viewMarkers.empty()) {
            collectDecodeErrorMarkers(viewMarkers, blockLine, tokenLines, &nli);
            reportDecodeErrorMarkers(text::Line{nli.position.line.v - 1}, tokenLines, viewMarkers, context);
        }
    }
    {
        auto viewMarkers = ViewMarkers{};
        for (auto& err : nli.value.errors) {
            err.visitSome([&](const scanner::MixedIndentCharacter& mic) { viewMarkers.emplace_back(mic.input); });
        }
        if (viewMarkers.empty()) return;

        for (auto& t : blockLine.insignificants) {
            t.visitSome([&](const nesting::NewLineIndentation& onli) {
                if (onli.isTainted || !onli.input.isPartOf(tokenLines)) return;
                for (auto& err : onli.value.errors) {
                    if (!err.holds<scanner::MixedIndentCharacter>()) return;
                }
                for (auto& err : onli.value.errors) {
                    err.visitSome(
                        [&](const scanner::MixedIndentCharacter& mic) { viewMarkers.emplace_back(mic.input); });
                }
                if (&onli != (void*)&nli) const_cast<nesting::NewLineIndentation&>(onli).isTainted = true;
            });
        }

        auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

        auto highlights = Highlights{};
        for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

        auto doc = Document{{Paragraph{String{"The indentation mixes tabs and spaces."}, {}},
                             SourceCodeBlock{escapedLines, highlights, String{}, text::Line{nli.position.line.v - 1}}}};

        auto expl = Explanation{String("Mixed Indentation Characters"), doc};

        auto d = Diagnostic{Code{String{"rebuild-lexer"}, 3}, Parts{expl}};
        context.reportDiagnostic(std::move(d));
    }
}

template<class Context>
void reportUnexpectedCharacter(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedCharacter& uc, ContextApi<Context>& context) {
    if (uc.isTainted) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, uc.input);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.insignificants) {
        t.visitSome([&](const nesting::UnexpectedCharacter& ouc) {
            if (ouc.input.begin() >= tokenLines.begin() && ouc.input.end() <= tokenLines.end()) {
                viewMarkers.emplace_back(ouc.input);
                if (&ouc != (void*)&uc) const_cast<nesting::UnexpectedCharacter&>(ouc).isTainted = true;
            }
        });
    }

    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{
        {Paragraph{(viewMarkers.size() == 1)
                       ? String{"The tokenizer encountered a character that is not part of any Rebuild language token."}
                       : String{"The tokenizer encountered multiple characters that are not part of any Rebuild "
                                "language token."},
                   {}},
         SourceCodeBlock{escapedLines, highlights, String{}, uc.position.line}}};

    auto expl = Explanation{String("Unexpected characters"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 2}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

} // namespace parser
