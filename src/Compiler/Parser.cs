using System;
using System.Collections.Generic;
using REC.AST;
using REC.Scope;
using REC.Scanner;
using REC.Tools;

namespace REC
{
    public interface IParserObserver
    {
        void CompileTimeFlag(TextFileRange range);
        void Identifier(IIdentifierLiteral identifier, TextFileRange range);
        void NumericalLiteral(INumberLiteral numberLiteral, TextFileRange range);
        void StringLiteral(IStringLiteral stringLiteral, TextFileRange range);

        void Assignment(TextFileRange range);
        void FunctionDecl(TextFileRange range, int indent);
        void VarDecl(TextInputRange range, int indent);
        void DefineDecl(TextInputRange range, int indent);

        void StartArguments(TextFileRange range);
        void EndArguments();

        void StartBlock(TextFileRange range);
        void Expression(IExpression expression);
        void End(TextFileRange range);
    }

    public class OldParser
    {
        private readonly IndentationParser _indentation = new IndentationParser();

        private readonly IParserObserver _observer;


        public OldParser(IParserObserver observer) {
            _observer = observer;
        }

        public bool ParseFile(TextFile file) {
            var range = new TextInputRange {
                File = file
            };
            ParseBlock(range, null);
            if (range.IsEndValid) {
                // TODO: expected to parse all
                return false;
            }
            return true;
        }

        private void ParseBlock(TextInputRange range, IndentationParser.ILevel parentLevel) {
            _indentation.ParseNewline(range);
            var level = _indentation.CurrentLevel;
            while (range.IsEndValid) {
                if (_indentation.ParseNewline(range)) {
                    if (range.IsKeyword("end")) {
                        if (_indentation.CurrentLevel != parentLevel) {
                            // TODO: something went wrong
                        }
                        _observer.End(range);
                        range.Collapse();
                        return;
                    }
                    if (level != _indentation.CurrentLevel) {
                        // TODO: something went wrong
                        if (level.Column > _indentation.CurrentLevel.Column) {
                            return;
                        }
                    }
                }
                var isDeclaration = ParseDeclaration(range, level);
                if (isDeclaration) continue;
                var isExpression = ParseLeftExpression(range, level);
                if (isExpression) continue;
                // TODO: syntax error
            }
            // TODO: missing end!
        }

        private bool ParseLeftExpression(TextInputRange range, IndentationParser.ILevel parentLevel) {
            if (range.IsKeyword("(")) {
                var isBracket = ParseLeftBracket(range, parentLevel);
                if (!isBracket) return false;
                return ParseRightExpression(range, parentLevel);
            }
            var isLiteral = ParseLiteral(range);
            if (isLiteral) {
                return ParseRightExpression(range, parentLevel);
            }
            var identifier = ParseIdentifier(range);
            if (identifier != null) {
                return ParseRightOfIdentifier((dynamic) identifier, range, parentLevel);
            }
            return false;
        }

        private bool ParseLeftBracket(TextInputRange range, IndentationParser.ILevel parentLevel) {
            throw new NotImplementedException();
        }

        private bool ParseRightExpression(TextInputRange range, IndentationParser.ILevel parentLevel) {
            return true;
        }

        private bool ParseLiteral(TextInputRange range) {
            var numberLiteral = NumberLiteralScanner.Scan(range);
            if (numberLiteral != null) {
                _observer.NumericalLiteral(numberLiteral, range);
                return true;
            }
            var stringLiteral = StringLiteralScanner.Scan(range);
            if (stringLiteral != null) {
                _observer.StringLiteral(stringLiteral, range);
                return true;
            }
            return false;
        }

        private IIdentifierLiteral ParseIdentifier(TextInputRange range) {
            var identifier = IdentifierScanner.Scan(range);
            if (identifier != null) {
                _observer.Identifier(identifier, range);
            }
            return identifier;
        }

        private bool ParseRightOfIdentifier(ITypedConstruct variable, TextInputRange range, IndentationParser.ILevel parentLevel) {
            //ParseRightExpression(variable, range);
            return false;
        }

        private bool ParseRightOfIdentifier(IModule module, TextInputRange range, IndentationParser.ILevel parentLevel) {
            //ParseRightExpression(variable, range);
            return false;
        }

        private bool ParseRightOfIdentifier(IFunction function, TextInputRange range, IndentationParser.ILevel parentLevel) {
            if (function.RightArguments.IsEmpty()) {
                return true; // no arguments required
            }
            return false;
            //return ParseArguments(function.RightArguments, range, parentLevel);
        }

        private bool ParseArguments(ICollection<IArgument> rightArguments, TextInputRange range, IndentationParser.ILevel parentLevel) {
            var isLiteral = ParseLiteral(range);
            if (isLiteral) {
                return ParseRightExpression(range, parentLevel);
            }
            return true;
        }

        private static void ScanComment(TextInputRange range) {
            while (range.IsEndValid && !range.IsEndNewline) range.Extend();
            // newline will collapse the range
        }

        private bool ParseDeclaration(TextInputRange range, IndentationParser.ILevel parentLevel) {
            if (range.IsKeyword("var")) {
                _observer.VarDecl(range, parentLevel.Column);
                range.CollapseWhitespaces();
                // TODO
                return true;
            }
            if (range.IsKeyword("def")) {
                _observer.DefineDecl(range, parentLevel.Column);
                range.CollapseWhitespaces();
                // TODO
                return true;
            }
            if (range.IsKeyword("fn")) {
                _observer.FunctionDecl(range, parentLevel.Column);
                range.CollapseWhitespaces();
                if (range.EndChar == '(') {
                    _observer.StartArguments(range);
                    // TODO
                    if (range.EndChar != ')') {
                        // ERROR
                    }
                    _observer.EndArguments();
                    range.CollapseWhitespaces();
                }
                if (range.EndChar == '&') {
                    _observer.CompileTimeFlag(range);
                    range.CollapseWhitespaces();
                }
                // TODO entry
                range.CollapseWhitespaces();
                _observer.StartArguments(range);
                if (range.EndChar == '(') {
                    // TODO
                    if (range.EndChar != ')') {
                        // ERROR
                    }

                    range.CollapseWhitespaces();
                }
                else {}
                _observer.EndArguments();
                if (range.IsKeyword("->")) {
                    _observer.StartArguments(range);
                    // TODO
                    _observer.EndArguments();
                }
                if (range.EndChar == ';') {
                    return true;
                }
                if (range.EndChar == '=') {
                    _observer.Assignment(range);
                    // add expression
                    return true;
                }
                _observer.StartBlock(range);
                return true;
            }
            return false;
        }
    }
}