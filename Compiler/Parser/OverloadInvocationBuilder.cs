using REC.AST;
using REC.Instance;
using REC.Tools;

namespace REC.Parser
{
    public interface ITokenToExpressionParser
    {
        bool IsActive { get; }
        bool IsCompletable { get; }

        void RetireToken(IExpression token);

        INamedExpression Build();
    }

    public interface IOverloadInvocationBuilder
    {
        bool IsActive { get; }

        // TODO: add reasons for being inactive
        bool IsCompletable { get; } // has all mandatory right arguments

        void RetireLeftArguments(INamedExpressionTuple leftArguments); // TODO: allow only single left argument
        void RetireRightArgument(INamedExpression argument);

        ITokenToExpressionParser[] NextRightArgumentParsers();

        IFunctionInvocation Build();
    }

    class OverloadInvocationBuilder : IOverloadInvocationBuilder
    {
        public bool IsActive { get; private set; }
        public bool IsCompletable { get; private set; }

        IFunctionInstance FunctionIntance { get; }
        TextFileRange Range { get; }
        INamedExpressionTuple LeftArguments { get; set; }
        INamedExpressionTuple RightArguments { get; } = new NamedExpressionTuple();

        public OverloadInvocationBuilder(IFunctionInstance instance, TextFileRange range) {
            FunctionIntance = instance;
            Range = range;
        }

        public void RetireLeftArguments(INamedExpressionTuple leftArguments) {
            if (!DoLeftArgumentsMatch(leftArguments)) {
                IsActive = false;
                return;
            }
            LeftArguments = leftArguments;
            IsActive = true;
            UpdateRightArgumentState();
        }

        bool DoLeftArgumentsMatch(INamedExpressionTuple leftArguments) {
            var f = FunctionIntance;
            // TODO: do not use declaration!
            if (f.Declaration.LeftArguments == null) return true;
            if (f.Declaration.LeftArguments.Count > leftArguments.Tuple.Count) return false;
            var o = leftArguments.Tuple.Count - f.Declaration.LeftArguments.Count;
            foreach (var fArg in f.Declaration.LeftArguments) {
                var givenArg = leftArguments.Tuple[o];
                if (!CanImplicitConvertExpressionTo(givenArg.Expression, fArg.Type)) return false;
                o++;
            }
            return f.Declaration.LeftArguments.Count <= leftArguments.Tuple.Count;
        }

        public void RetireRightArgument(INamedExpression argument) {
            // TODO: Check Convertable!
            RightArguments.Tuple.Add(argument);
            UpdateRightArgumentState();
        }

        void UpdateRightArgumentState() {
            if (FunctionIntance.RightArguments == null) {
                IsActive = false;
                IsCompletable = true;
                return;
            }
            if (FunctionIntance.Declaration.MandatoryRightArgumentCount() <= RightArguments.Tuple.Count) {
                IsCompletable = true;
            }
            if (FunctionIntance.Declaration.MaxRightArgumentCount() == RightArguments.Tuple.Count) {
                IsActive = false;
            }
        }

        public ITokenToExpressionParser[] NextRightArgumentParsers() {
            return new ITokenToExpressionParser[] { };
        }

        public IFunctionInvocation Build() {
            return new FunctionInvocation {
                Function = FunctionIntance,
                Range = Range,
                Left = AssignArguments(FunctionIntance.LeftArguments, LeftArguments),
                Right = AssignArguments(FunctionIntance.RightArguments, RightArguments)
            };
        }

        static INamedExpressionTuple AssignArguments(NamedCollection<IArgumentInstance> instances, INamedExpressionTuple arguments) {
            var result = new NamedExpressionTuple();
            var o = arguments.Tuple.Count - instances.Count;
            foreach (var instance in instances) {
                // TODO: check that conversion exists
                result.Tuple.Add(arguments.Tuple[o]);
                arguments.Tuple.RemoveAt(o);
            }
            // TODO: check that enough arguments are given
            return result;
        }

        static bool CanImplicitConvertExpressionTo(IExpression givenArgExpression, IModuleInstance fArgType) {
            return true;
        }
    }
}
