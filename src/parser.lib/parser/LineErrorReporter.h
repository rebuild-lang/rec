#pragma once
#include "Context.h"

#include "nesting/Token.h"

#include "strings/Rope.h"
#include "strings/utf8Decode.h"

#include <bitset>
#include <sstream>

namespace parser {

template<class Context>
void reportLineErrors(const nesting::BlockLine& line, Context& context) {
    line.forEach([&](auto& t) {
        t.visitSome(
            // scanner
            [&](const nesting::NewLineIndentation& nli) { reportNewline(line, nli, context); },
            [&](const nesting::CommentLiteral& cl) { reportTokenWithDecodeErrors(line, cl, context); },
            [&](const nesting::StringLiteral& sl) { reportStringLiteral(line, sl, context); },
            [&](const nesting::NumberLiteral& sl) { reportNumberLiteral(line, sl, context); },
            [&](const nesting::IdentifierLiteral& il) { reportTokenWithDecodeErrors(line, il, context); },
            [&](const nesting::OperatorLiteral& ol) { reportOperatorLiteral(line, ol, context); },
            [&](const nesting::InvalidEncoding& ie) { reportInvalidEncoding(line, ie, context); },
            [&](const nesting::UnexpectedCharacter& uc) { reportUnexpectedCharacter(line, uc, context); },
            // filter
            [&](const nesting::UnexpectedColon& uc) { reportUnexpectedColon(line, uc, context); },
            // nesting
            [&](const nesting::UnexpectedIndent& ui) { reportUnexpectedIndent(line, ui, context); },
            [&](const nesting::UnexpectedTokensAfterEnd& utae) { reportUnexpectedTokenAfterEnd(line, utae, context); },
            [&](const nesting::UnexpectedBlockEnd& ube) { reportUnexpectedBlockEnd(line, ube, context); },
            [&](const nesting::MissingBlockEnd& mbe) { reportMissingBlockEnd(line, mbe, context); });
    });
}

inline auto extractBlockLines(const nesting::BlockLine& blockLine) -> strings::View {
    auto begin = strings::View::It{};
    auto end = strings::View::It{};
    blockLine.forEach([&](auto& g) {
        auto begin2 = g.visit([](auto& t) { return t.input.begin(); });
        auto end2 = g.visit([](auto& t) { return t.input.end(); });
        if (!begin || (begin2 && begin2 < begin)) begin = begin2;
        if (!end || (end2 && end2 > end)) end = end2;
    });
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
            i++;
        }
        return EscapedMarkers{to_string(view), std::move(markers)};
    }

    return EscapedMarkers{to_string(output), std::move(markers)};
}

template<class ContextBase>
void reportDecodeErrorMarkers(
    text::Line line, strings::View tokenLines, const parser::ViewMarkers& viewMarkers, Context<ContextBase>& context) {

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
void reportDecodeErrors(const nesting::BlockLine& blockLine, const Token& tok, Context& context) {
    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, tok.input);
    auto viewMarkers = ViewMarkers{};
    collectDecodeErrorMarkers(viewMarkers, blockLine, tokenLines, &tok);
    reportDecodeErrorMarkers(tok.position.line, tokenLines, viewMarkers, context);
}

template<class ContextBase>
void reportNewline(
    const nesting::BlockLine& blockLine, const nesting::NewLineIndentation& nli, Context<ContextBase>& context) {
    if (nli.isTainted || !nli.value.hasErrors()) return; // already reported or no errors

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, nli.input);
    {
        auto viewMarkers = ViewMarkers{};
        for (auto& err : nli.value.errors) {
            err.visitSome([&](const scanner::DecodedErrorPosition& dep) { viewMarkers.emplace_back(dep.input); });
        }
        if (!viewMarkers.empty()) {
            if (viewMarkers.size() == nli.value.errors.size()) viewMarkers.clear();
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

template<class... Tags, class Context>
void reportTokenWithDecodeErrors(
    const nesting::BlockLine& blockLine,
    const scanner::details::TagTokenWithDecodeErrors<Tags...>& de,
    Context& context) {
    if (de.isTainted || de.decodeErrors.empty()) return; // already reported or no errors

    reportDecodeErrors(blockLine, de, context);
}

template<class ContextBase>
void reportStringLiteral(
    const nesting::BlockLine& blockLine, const nesting::StringLiteral& sl, Context<ContextBase>& context) {
    if (sl.isTainted || !sl.value.hasErrors()) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, sl.input);

    auto reportedKinds = std::bitset<8>{};
    for (auto& err : sl.value.errors) {
        if (reportedKinds[static_cast<int>(err.kind)]) continue;
        reportedKinds.set(static_cast<int>(err.kind));

        auto viewMarkers = ViewMarkers{};
        for (auto& err2 : sl.value.errors)
            if (err2.kind == err.kind) viewMarkers.emplace_back(err2.input);

        auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

        auto highlights = Highlights{};
        for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

        using Kind = scanner::StringError::Kind;
        switch (err.kind) {
        case Kind::EndOfInput: {
            auto doc = Document{{Paragraph{String{"The string was not terminated."}, {}},
                                 SourceCodeBlock{escapedLines, highlights, String{}, sl.position.line}}};
            auto expl = Explanation{String("Unexpected end of input"), doc};
            auto d = Diagnostic{Code{String{"rebuild-lexer"}, 10}, Parts{expl}};
            context.reportDiagnostic(std::move(d));
            break;
        }
        case Kind::InvalidEncoding: {
            reportDecodeErrorMarkers(sl.position.line, tokenLines, viewMarkers, context);
            break;
        }
        case Kind::InvalidEscape: {
            auto doc = Document{{Paragraph{String{"These Escape sequences are unknown."}, {}},
                                 SourceCodeBlock{escapedLines, highlights, String{}, sl.position.line}}};
            auto expl = Explanation{String("Unkown escape sequence"), doc};
            auto d = Diagnostic{Code{String{"rebuild-lexer"}, 11}, Parts{expl}};
            context.reportDiagnostic(std::move(d));
            break;
        }
        case Kind::InvalidControl: {
            auto doc = Document{{Paragraph{String{"Use of invalid control characters. Use escape sequences."}, {}},
                                 SourceCodeBlock{escapedLines, highlights, String{}, sl.position.line}}};
            auto expl = Explanation{String("Unkown control characters"), doc};
            auto d = Diagnostic{Code{String{"rebuild-lexer"}, 12}, Parts{expl}};
            context.reportDiagnostic(std::move(d));
            break;
        }
        case Kind::InvalidDecimalUnicode: {
            auto doc = Document{{Paragraph{String{"Use of invalid decimal unicode values."}, {}},
                                 SourceCodeBlock{escapedLines, highlights, String{}, sl.position.line}}};
            auto expl = Explanation{String("Invalid decimal unicode"), doc};
            auto d = Diagnostic{Code{String{"rebuild-lexer"}, 13}, Parts{expl}};
            context.reportDiagnostic(std::move(d));
            break;
        }
        case Kind::InvalidHexUnicode: {
            auto doc = Document{{Paragraph{String{"Use of invalid hexadecimal unicode values."}, {}},
                                 SourceCodeBlock{escapedLines, highlights, String{}, sl.position.line}}};
            auto expl = Explanation{String("Invalid hexadecimal unicode"), doc};
            auto d = Diagnostic{Code{String{"rebuild-lexer"}, 14}, Parts{expl}};
            context.reportDiagnostic(std::move(d));
            break;
        }
        } // switch
    }

    auto viewMarkers = ViewMarkers{};
}

template<class ContextBase>
void reportNumberLiteral(
    const nesting::BlockLine& blockLine, const nesting::NumberLiteral& nl, Context<ContextBase>& context) {
    if (nl.isTainted || !nl.value.hasErrors()) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, nl.input);

    auto reportedKinds = std::bitset<scanner::NumberLiteralError::optionCount()>{};
    for (auto& err : nl.value.errors) {
        auto kind = err.index().value();
        if (reportedKinds[kind]) continue;
        reportedKinds.set(kind);

        auto viewMarkers = ViewMarkers{};
        for (auto& err2 : nl.value.errors)
            if (err2.index() == err.index()) err2.visit([&](auto& v) { viewMarkers.emplace_back(v.input); });

        auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

        auto highlights = Highlights{};
        for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

        err.visit(
            [&](const scanner::DecodedErrorPosition&) {
                reportDecodeErrorMarkers(nl.position.line, tokenLines, viewMarkers, context);
            },
            [&, &escapedLines = escapedLines](const scanner::NumberMissingExponent&) {
                auto doc = Document{{Paragraph{String{"After the exponent sign an actual value is expected."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, nl.position.line}}};
                auto expl = Explanation{String("Missing exponent value"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 20}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            },
            [&, &escapedLines = escapedLines](const scanner::NumberMissingValue&) {
                auto doc = Document{{Paragraph{String{"After the radix sign an actual value is expected."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, nl.position.line}}};
                auto expl = Explanation{String("Missing value"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 21}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            },
            [&, &escapedLines = escapedLines](const scanner::NumberMissingBoundary&) {
                auto doc = Document{{Paragraph{String{"The number literal ends with an unknown suffix."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, nl.position.line}}};
                auto expl = Explanation{String("Missing boundary"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 22}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            });
    }
}

template<class ContextBase>
void reportOperatorLiteral(
    const nesting::BlockLine& blockLine, const nesting::OperatorLiteral& ol, Context<ContextBase>& context) {
    if (ol.isTainted || !ol.value.hasErrors()) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, ol.input);

    auto reportedKinds = std::bitset<scanner::OperatorLiteralError::optionCount()>{};
    for (auto& err : ol.value.errors) {
        auto kind = err.index().value();
        if (reportedKinds[kind]) continue;
        reportedKinds.set(kind);

        auto viewMarkers = ViewMarkers{};
        for (auto& err2 : ol.value.errors)
            if (err2.index() == err.index()) err2.visit([&](auto& v) { viewMarkers.emplace_back(v.input); });

        auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

        auto highlights = Highlights{};
        for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

        err.visit(
            [&](const scanner::DecodedErrorPosition&) {
                reportDecodeErrorMarkers(ol.position.line, tokenLines, viewMarkers, context);
            },
            [&, &escapedLines = escapedLines](const scanner::OperatorWrongClose&) {
                auto doc = Document{{Paragraph{String{"The closing sign does not match the opening sign."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, ol.position.line}}};
                auto expl = Explanation{String("Operator wrong close"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 30}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            },
            [&, &escapedLines = escapedLines](const scanner::OperatorUnexpectedClose&) {
                auto doc = Document{{Paragraph{String{"There was no opening sign before the closing sign."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, ol.position.line}}};
                auto expl = Explanation{String("Operator unexpected close"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 31}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            },
            [&, &escapedLines = escapedLines](const scanner::OperatorNotClosed&) {
                auto doc = Document{{Paragraph{String{"The operator ends before the closing sign was found."}, {}},
                                     SourceCodeBlock{escapedLines, highlights, String{}, ol.position.line}}};
                auto expl = Explanation{String("Operator not closed"), doc};
                auto d = Diagnostic{Code{String{"rebuild-lexer"}, 32}, Parts{expl}};
                context.reportDiagnostic(std::move(d));
            });
    }
}

template<class Context>
void reportInvalidEncoding(const nesting::BlockLine& blockLine, const nesting::InvalidEncoding& ie, Context& context) {
    if (ie.isTainted) return; // already reported

    reportDecodeErrors(blockLine, ie, context);
}

template<class ContextBase>
void reportUnexpectedCharacter(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedCharacter& uc, Context<ContextBase>& context) {
    if (uc.isTainted) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, uc.input);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.insignificants) {
        t.visitSome([&](const nesting::UnexpectedCharacter& ouc) {
            if (ouc.input.isPartOf(tokenLines)) {
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

template<class ContextBase>
void reportUnexpectedColon(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedColon& uc, Context<ContextBase>& context) {
    if (uc.isTainted) return;

    using namespace diagnostic;

    auto tokenLines = extractViewLines(blockLine, uc.input);

    auto viewMarkers = ViewMarkers{uc.input};
    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{{Paragraph{String{"The colon cannot be the only token on a line."}, {}},
                         SourceCodeBlock{escapedLines, highlights, String{}, uc.position.line}}};

    auto expl = Explanation{String("Unexpected colon"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 4}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

template<class ContextBase>
void reportUnexpectedIndent(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedIndent& ui, Context<ContextBase>& context) {
    if (ui.isTainted) return;

    using namespace diagnostic;

    // TODO(arBmind): find a way to add the line before.
    // we probably have to report this in the parent block
    auto tokenLines = extractBlockLines(blockLine);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.insignificants) {
        t.visitSome([&](const nesting::UnexpectedIndent& oui) {
            if (oui.input.isPartOf(tokenLines)) {
                viewMarkers.emplace_back(oui.input);
                if (&oui != (void*)&ui) const_cast<nesting::UnexpectedIndent&>(oui).isTainted = true;
            }
        });
    }

    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{
        {Paragraph{String{"The indentation is above the regular block level, but does not leave the block."}, {}},
         SourceCodeBlock{escapedLines, highlights, String{}, ui.position.line}}};

    auto expl = Explanation{String("Unexpected indent"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 5}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

template<class ContextBase>
void reportUnexpectedTokenAfterEnd(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedTokensAfterEnd& utae, Context<ContextBase>& context) {

    using namespace diagnostic;
    auto tokenLines = extractBlockLines(blockLine);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.tokens) {
        t.visit([&](const auto& tok) {
            if (tok.position >= utae.position && tok.input.isPartOf(tokenLines)) {
                viewMarkers.emplace_back(tok.input);
            }
        });
    }

    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{{Paragraph{String{"After end no more tokens are allowed."}, {}},
                         SourceCodeBlock{escapedLines, highlights, String{}, utae.position.line}}};

    auto expl = Explanation{String("Unexpected tokens after end"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 6}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

template<class ContextBase>
void reportUnexpectedBlockEnd(
    const nesting::BlockLine& blockLine, const nesting::UnexpectedBlockEnd& ube, Context<ContextBase>& context) {

    if (ube.isTainted) return;
    using namespace diagnostic;
    auto tokenLines = extractBlockLines(blockLine);

    auto viewMarkers = ViewMarkers{};
    for (auto& t : blockLine.insignificants) {
        t.visitSome([&](const nesting::UnexpectedBlockEnd& oube) {
            if (oube.input.isPartOf(tokenLines)) {
                viewMarkers.emplace_back(oube.input);
                if (&oube != (void*)&ube) const_cast<nesting::UnexpectedBlockEnd&>(oube).isTainted = true;
            }
        });
    }

    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{{Paragraph{String{"The end keyword is only allowed to end blocks"}, {}},
                         SourceCodeBlock{escapedLines, highlights, String{}, ube.position.line}}};

    auto expl = Explanation{String("Unexpected block end"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 7}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

template<class ContextBase>
void reportMissingBlockEnd(
    const nesting::BlockLine& blockLine, const nesting::MissingBlockEnd& ube, Context<ContextBase>& context) {

    if (ube.isTainted) return;
    using namespace diagnostic;
    auto tokenLines = extractBlockLines(blockLine);

    auto viewMarkers = ViewMarkers{};
    auto [escapedLines, escapedMarkers] = escapeSourceLine(tokenLines, viewMarkers);

    auto highlights = Highlights{};
    for (auto& m : escapedMarkers) highlights.emplace_back(Marker{m, {}});

    auto doc = Document{{Paragraph{String{"The block ended without the end keyword"}, {}},
                         SourceCodeBlock{escapedLines, highlights, String{}, ube.position.line}}};

    auto expl = Explanation{String("Missing Block End"), doc};

    auto d = Diagnostic{Code{String{"rebuild-lexer"}, 8}, Parts{expl}};
    context.reportDiagnostic(std::move(d));
}

} // namespace parser
