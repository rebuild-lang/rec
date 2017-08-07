
using REC.Packaging.Code;
using REC.Packaging.x86;
using System.IO;

namespace REC
{
    public static class Program
    {
        public static void Main(string[] args) {
            var entry = new Label { Name = "Entry" };
            var import = new ImportDll { Name = "kernel32.dll" };
            var exitProcess = import.AddNamed("ExitProcess", 346);

            var executable = new Packaging.Executable {
                Name = "Test Executable",
                Version = new Packaging.Version { Major = 1, Minor = 0 },
                Instructions = {
                    entry,
                    new ImmediateInstruction { Type = ImmediateInstructionType.Push, Immediate = NativeValue.Create((byte)5) },
                    //new ImmediateInstruction { Type = ImmediateInstructionType.CallRelative, Immediate = NativeValue.Create((uint)0) },
                    new DllEntryInstruction { DllEntry = exitProcess }
                },
                EntryLabel = entry,
                DllImports = {
                    import
                }
            };

            var image = new Packaging.PortableExecutable.Image(executable);
            using (var writer = File.Create("C:\\Rebuild\\test.exe"))
            {
                using (var bw = new BinaryWriter(writer))
                {
                    image.Write(bw);
                }
            }

            /*
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
    fn test (x : u64):
        Rebuild.Print x
    end
end

fn (*l : u64) += (r : u64):
    l = l + r
end

let & c = Rebuild.Compiler
Rebuild.AddCode c, """"""
    #let *x : u64
    #x = 23 + 12
    #x += 5
    test 5
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
            */
        }
    }
}
