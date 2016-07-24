using System;
using REC.AST;

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
        void End(TextFileRange range);
    }

    public interface IParserState
    {
        void IdentifierScope(IdentifierScope scope);
    }

    public class Parser
    {
        private readonly IParserObserver _observer;

        public Parser(IParserObserver observer) {
            _observer = observer;
        }

        public bool Parse(TextFile file) {
            var range = new TextInputRange {
                File = file
            };
            var isIndent = true;
            var indent = 0;

            Action handleNewLine = () => {
                range.NewLine();
                range.Collapse();
                isIndent = true;
            };

            Func<bool> handleDeclarations = () => {
                if (range.IsKeyword("var")) {
                    _observer.VarDecl(range, indent);
                    range.CollapseWhitespaces();
                    // TODO
                    return true;
                }
                if (range.IsKeyword("def")) {
                    _observer.DefineDecl(range, indent);
                    range.CollapseWhitespaces();
                    // TODO
                    return true;
                }
                if (range.IsKeyword("fn")) {
                    _observer.FunctionDecl(range, indent);
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
            };

            while (range.IsEndValid) {
                if (range.IsEndNewline) {
                    handleNewLine();
                    continue;
                }
                if (range.IsEndWhitespace) {
                    range.CollapseWhitespaces();
                    continue;
                }
                if (isIndent) {
                    indent = range.Start.Column;
                    isIndent = false;
                    if (handleDeclarations()) continue;
                }
                var chr = range.EndChar;
                switch (chr) {
                    case '#':
                        break;
                    case '(':
                        break;
                    case ')':
                        break;
                    case '[':
                        break;
                    case ']':
                        break;
                    case ',':
                        break;
                    case '.':
                        break;
                    default:
                        break;
                }
            }
            return !range.IsEndValid;
        }
    }
}