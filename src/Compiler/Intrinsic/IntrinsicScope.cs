using System.Collections;
using System.Collections.Generic;
using REC.AST;
using REC.Scope;
using REC.Tools;

namespace REC.Intrinsic
{
    using Dict = Dictionary<string, IIntrinsic>;

    public interface IIntrinsicExpression : IExpression
    {
        IIntrinsic Intrinsic { get; }
    }

    class IntrinsicExpression : Expression, IIntrinsicExpression
    {
        public IIntrinsic Intrinsic { get; set; }
    }

    class IntrinsicScope : IEnumerable
    {
        private readonly Dict _identifiers = new Dict();

        public bool Add(IIntrinsic entry) {
            var label = entry.Name;
            if (_identifiers.ContainsKey(label))
                return false;
            _identifiers.Add(label, entry);
            return true;
        }

        // this is only to allow brace initialization
        public IEnumerator GetEnumerator() {
            return _identifiers.GetEnumerator();
        }

        public IScope Build() {
            var result = new Scope.Scope();
            foreach (var pair in _identifiers) {
                var intrinsic = pair.Value;
                var functionDecl = new FunctionDeclaration {
                    Name = intrinsic.Name,
                    RightArguments = intrinsic.RightArguments,
                    Results = intrinsic.Results,
                    Implementation = new ExpressionBlock()
                };
                functionDecl.Implementation.Expressions.Add(new IntrinsicExpression { Intrinsic = intrinsic });
                var functionEntry = new FunctionEnty();
                functionEntry.FunctionDeclarations.Add(functionDecl);
                result.Add(functionEntry);
            }
            return result;
        }
    }
}
