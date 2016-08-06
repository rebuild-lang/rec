using System;
using REC.AST;
using REC.Identifier;
using REC.Scanner;
using IModule = REC.Identifier.IModule;

namespace REC
{
    public interface IParserObserver
    {
        void CompileTimeFlag(TextFileRange range);
        void Identifier(TextFileRange range);
        void NumericalLiteral(INumberLiteral numberLiteral, TextFileRange range);
        void StringLiteral(string text, TextFileRange range);

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

    public interface IParserState
    {
        void IdentifierScope(IIdentifierScope scope);
    }

    public class Parser
    {
        private readonly IndentationParser indentation = new IndentationParser();

        private readonly IParserObserver _observer;
        private IIdentifierScope _scope;
        private IdentifierScanner _identifierScanner;


        public Parser(IParserObserver observer) {
            _observer = observer;
            _scope = new IdentifierScope();
            UpdateIdentifierScanner();
        }

        private void UpdateIdentifierScanner() {
            _identifierScanner = new IdentifierScanner();
            foreach (var id in _scope) _identifierScanner.Add(id);
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
            indentation.ParseNewline(range);
            var level = indentation.CurrentLevel;
            while (range.IsEndValid) {
                if (indentation.ParseNewline(range)) {
                    if (range.IsKeyword("end")) {
                        if (indentation.CurrentLevel != parentLevel) {
                            // TODO: something went wrong
                        }
                        range.Collapse();
                        return;
                    }
                    if (level != indentation.CurrentLevel) {
                        // TODO: something went wrong
                        if (level.Column > indentation.CurrentLevel.Column) {
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
            return false;
        }

        private void ParseIdentifier(ITypedConstruct variable, TextInputRange range) {
            //ParseRightExpression(variable, range);
        }
        private void ParseIdentifier(IModule module, TextInputRange range) {
            //ParseRightExpression(variable, range);
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
                // TODO identifier
                range.CollapseWhitespaces();
                _observer.StartArguments(range);
                if (range.EndChar == '(') {
                    // TODO
                    if (range.EndChar != ')') {
                        // ERROR
                    }

                    range.CollapseWhitespaces();
                }
                else {
                }
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