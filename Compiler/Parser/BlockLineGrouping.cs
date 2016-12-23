using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Scanner;

namespace REC.Parser
{
    /**
         * Preparation step that allows to defer block processing
         * 
         * * Lines are joined with all continuations
         * * BlockStart gets ITokenBlock as Data containing all lines
         * * Semicolons are splitten into separate lines
         * 
         * example:S
         *   'if' 'a'[NewLine
         *      ]'&&' 'b'[BlockStart
         *     ]'print' "a"[NewLine
         *   ]'else'[BlockStart
         *     ]'print' "b"[BlockEnd
         *   ]
         * 
         *  is turned into:
         *   Lines:
         *   - - 'if'
         *     - 'a'
         *     - '&&'
         *     - 'b'
         *     - BlockStart:
         *         Lines:
         *         - - 'print'
         *           - "a"
         *     - 'else'
         *     - BlockStart:
         *         Lines:
         *         - - 'print'
         *           - "b"
         */

    public class BlockLineGrouping
    {
        bool _done; // marks that we hit the end of input
        char _indentChar = '\0';

        public IBlockLiteral Group(IEnumerable<TokenData> input) {
            _indentChar = '\0';
            _done = false;
            using (var it = input.GetEnumerator()) {
                if (!it.MoveNext()) return new BlockLiteral();
                var blockColumn = 1;
                if (it.Current.Type == Token.NewLineIndentation
                    || it.Current.Type == Token.WhiteSpaceSeperator) {
                    // first whitespace is an indentation
                    blockColumn = GetIndent(it);
                    if (!it.MoveNext()) return new BlockLiteral();
                }
                var block = ParseBlock(it, blockColumn);
                if (!_done) {
                    // TODO: report extra lines
                }
                return block;
            }
        }

        IBlockLiteral ParseBlock(IEnumerator<TokenData> it, int blockColumn) {
            var block = new BlockLiteral();
            while (true) {
                while (true) {
                    int indent;
                    switch (it.Current.Type) {
                        case Token.BlockEndIndentation:
                            indent = GetIndent(it);
                            if (indent < blockColumn) return block; // do not consume parent end block
                            // TODO report misplaced end
                            // handling: ignore it
                            if (!it.MoveNext()) {
                                _done = true;
                                return block;
                            }
                            continue;
                        case Token.BlockStartIndentation: // this occurs if block starts with the wrong indentation
                        case Token.NewLineIndentation:
                            indent = GetIndent(it);
                            if (indent < blockColumn) return block; // line is not part of this block
                            if (indent > blockColumn) {
                                // TODO: report indentation error
                                // handling: take the line into this block
                            }
                            if (!it.MoveNext()) {
                                _done = true;
                                return block;
                            }
                            continue;
                    }
                    break;
                }
                var line = ParseLine(it, blockColumn);
                if (line != null) block.Lines.Add(line);
                if (_done) break;
            }
            return block;
        }

        ITokenLine ParseLine(IEnumerator<TokenData> it, int parentBlockColumn) {
            var line = new TokenLine();
            var expectEnd = false;
            while (true) {
                ExtractLineTokens(it, line);
                if (_done) return line;

                while (true) {
                    var nextColumn = GetIndent(it);
                    switch (it.Current.Type) {
                        case Token.NewLineIndentation:
                            if (nextColumn < parentBlockColumn) return line; // end of the parent block
                            if (nextColumn == parentBlockColumn && !expectEnd) return line; // normal line break
                            // line continuation
                            while (true) {
                                if (!it.MoveNext()) _done = true;
                                if (_done) return line; // should not happen (final NewLineIndentation was filtered)
                                ExtractLineTokens(it, line);
                                if (_done) return line;

                                if (it.Current.Type != Token.NewLineIndentation) break;
                                //var continueColumn = GetIndent(it);
                                //if (continueColumn >= nextColumn) continue; // next line is part of the continuation
                                // TODO report continuation error
                                // TODO handling: add lines to a block as well
                            }
                            continue;
                        case Token.BlockEndIndentation:
                            if (nextColumn < parentBlockColumn) {
                                if (expectEnd) {
                                    // TODO report missing end
                                    // handling terminate line anyways
                                }
                                return line; // end of the parent block
                            }
                            if (nextColumn == parentBlockColumn) {
                                if (!expectEnd) {
                                    // TODO report unexpected end
                                    // handling terminate line anyways
                                }
                                if (!it.MoveNext()) _done = true; // consume the end
                                return line;
                            }
                            // TODO report nested end
                            // handling: ignored
                            if (!it.MoveNext()) {
                                _done = true;
                                return line;
                            }
                            continue;
                        case Token.BlockStartIndentation:
                            expectEnd = true;
                            var current = it.Current;
                            if (nextColumn < parentBlockColumn) {
                                // TODO report missing end
                                // handling: add empty block and finish line
                                current.Data = new BlockLiteral();
                                line.Tokens.Add(current);
                                return line;
                            }
                            if (nextColumn == parentBlockColumn) {
                                // empty block
                                current.Data = new BlockLiteral();
                                line.Tokens.Add(current);
                                if (!it.MoveNext()) _done = true;
                                continue;
                            }
                            current.Data = ParseBlock(it, nextColumn);
                            line.Tokens.Add(current);
                            continue;
                    }
                    break;
                }
            }
        }

        void ExtractLineTokens(IEnumerator<TokenData> it, ITokenLine line) {
            var current = it.Current;
            while (current.Type != Token.NewLineIndentation
                && current.Type != Token.BlockStartIndentation
                && current.Type != Token.BlockEndIndentation) {
                line.Tokens.Add(current);
                if (!it.MoveNext()) {
                    _done = true;
                    return;
                }
                current = it.Current;
            }
        }

        int GetIndent(IEnumerator<TokenData> it) {
            var range = it.Current.Range;
            if (range == null) return 1;
            if (range.Length != 0) {
                var text = range.Text;
                if (_indentChar == '\0') _indentChar = text[index: 0];
                if (text.Any(x => x != _indentChar)) {} // TODO warning
            }
            return range.End.Column;
        }
    }
}
