using REC.Intrinsic;
using REC.Scope;

namespace REC
{
    class Compiler
    {
        public Compiler() {
            IntrinsicScope = new Scope.Scope {new PrintIntrinsic()};
        }

        IScope IntrinsicScope { get; }

        public void CompileFile(TextFile file) {
            var unfiltered = Scanner.Scanner.ScanFile(file);
            var commentFiltered = Parser.CommentIndentationFilter.Filter(unfiltered);
            using (var tokenIt = commentFiltered.GetEnumerator()) {
                if (tokenIt.MoveNext()) {
                    var block = Parser.Parser.ParseBlock(tokenIt, IntrinsicScope);
                }
            }
        }
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = "print \"Hello\"",
                    Filename = "Test.rebuild"
                });
        }
    }
}