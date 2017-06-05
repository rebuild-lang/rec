namespace REC
{
    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.AddTextFile(
                new TextFile {
                    Content = @"
phase Expr:
    fn (*l : u64) = (r : u64):
        Rebuild.Assign l, r
    end
    fn (a : u64) + (b : u64) -> *r : u64:
        r = Rebuild.Add a b
    end
    fn (*l : u64) += (r : u64):
        l = l + r
    end
    fn test (x : u64):
        Rebuild.Print x
    end
end

let & c = Rebuild.Compiler
Rebuild.AddCode c, """"""
    let *x : u64
    x = 23 + 12
    x += 5
    test x
""""""
#let& e = c.CompileTo Expr
#&e.Print

let & *y : u64
# Rebuild.Eval(y = 42 + 12)
# &Rebuild.Print y

Rebuild.Eval test Rebuild.Add 23 32
Rebuild.Print 42 + Rebuild.Eval(12 + 10)",
                    Filename = "Test.rebuild"
                });
            compiler.CompileToExecutable();
        }
    }
}
