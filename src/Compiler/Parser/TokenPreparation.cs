using System;
using System.Collections.Generic;
using REC.Scanner;

namespace REC.Parser
{
    public static class TokenPreparation
    {
        /**
         * Preparation step to ease the parsing of the token stream
         * 
         * filters out 
         * * invalid characters
         * * all comments + preceding whitespace
         * * newline + indentations preceding comments
         * * two adjacent newline + indentations (basically multiple newlines)
         * * ":" between two indentations (error)
         * mutates
         * * ":" before indentation => block start
         * * "end" after indentation => block end
         */
        public static IEnumerable<TokenData> Apply(IEnumerable<TokenData> input) {
            using (var it = input.GetEnumerator()) {
                do {
                    if (!it.MoveNext()) yield break;
                } while (it.Current.Type == Token.Comment
                         || it.Current.Type == Token.InvalidCharacter); // skip initial comments
                var lastYieldType = Token.NewLineIndentation;
                var previous = it.Current;
                while (it.MoveNext()) {
                    var current = it.Current;
                    if (previous.Type == Token.IdentifierLiteral && previous.Range.Text == ":") {
                        // this might be a block start, keep it in buffer
                        while (current.Type == Token.WhiteSpaceSeperator
                               || current.Type == Token.Comment) {
                            if (!it.MoveNext()) {
                                yield return previous; // report the regular token
                                yield break; // finished
                            }
                            current = it.Current;
                        }
                    }
                    switch (current.Type) {
                        case Token.Comment: // skip any comment
                            if (previous.Type != Token.NewLineIndentation // skip indentation before comment
                                && previous.Type != Token.WhiteSpaceSeperator) {
                                // skip whitespace before comment
                                lastYieldType = previous.Type;
                                yield return previous;
                            }
                            if (!it.MoveNext()) yield break; // skip final comment / indentation
                            current = it.Current;
                            if (previous.Type == Token.WhiteSpaceSeperator
                                && current.Type != Token.NewLineIndentation
                                && current.Type != Token.WhiteSpaceSeperator) {
                                lastYieldType = previous.Type;
                                yield return previous; // make sure no double whitespace is present
                            }
                            break;
                        case Token.NewLineIndentation:
                            if (previous.Type == Token.IdentifierLiteral && previous.Range.Text == ":") {
                                if (lastYieldType == Token.NewLineIndentation
                                    || lastYieldType == Token.BlockStartIndentation) {
                                    // TODO: report as error
                                    // handling: ignore it
                                }
                                else current.Type = Token.BlockStartIndentation;
                            }
                            else if (previous.Type != Token.NewLineIndentation) {
                                lastYieldType = previous.Type;
                                yield return previous; // skip double newlines
                            }
                            break;
                        case Token.IdentifierLiteral:
                            if (previous.Type == Token.NewLineIndentation && current.Range.Text == "end") {
                                previous.Type = Token.BlockEndIndentation; // NewLine + End = BlockEndIndentation
                                lastYieldType = previous.Type;
                                yield return previous;
                                if (!it.MoveNext()) yield break; // skip "end" token
                                current = it.Current;
                                break;
                            }
                            lastYieldType = previous.Type;
                            yield return previous;
                            break;
                        case Token.InvalidCharacter:
                            continue;
                        default:
                            lastYieldType = previous.Type;
                            yield return previous; // keep everything else
                            break;
                    }
                    previous = current;
                }
                if (previous.Type != Token.NewLineIndentation
                    && previous.Type != Token.WhiteSpaceSeperator) yield return previous;
            }
        }
    }
}