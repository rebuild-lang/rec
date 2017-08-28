using REC.Packaging.Code;
using REC.Packaging.Resource;
using REC.Packaging.x86;
using System.Collections.Generic;
using System.IO;

namespace REC
{
    public static class Program
    {
        public static void Main(string[] args) {
            var entry = new Label { Name = "Entry" };
            var import = new ImportDll { Name = "kernel32.dll" };
            var exitProcess = import.AddNamed("ExitProcess", 346);

            var image = new Packaging.PortableExecutable.Image {
                Header = {
                   ImageVersion = new Packaging.Version { Major = 1, Minor = 0 }
                },
                //Name = "Test Executable",
                Code = {
                    entry,
                    //new ImmediateInstruction(ImmediateInstructionType.Push, NativeValue.Create((byte)5)),
                    new ImmediateInstruction(ImmediateInstructionType.CallRelative, NativeValue.Create((uint)0)),
                    new DllEntryInstruction { DllEntry = exitProcess }
                },
                EntryLabel = entry,
                Imports = {
                    import
                },
                //Resources = {
                //    new IconParameters {
                //        Name = "DESK1",
                //        Stream = new FileStream("C:\\Rebuild\\main.ico", FileMode.Open)
                //    },
                //    new VersionParameters {
                //        FixedData = {
                //                    FileVersion = (1,2,3,4),
                //                    ProductVersion = (5,6,7,8),
                //                },
                //        StringTables = {
                //                    {
                //                        new VersionParameters.LanguageCodePage {
                //                            CodePage = CodePages.Unicode,
                //                            Language = Languages.UsEnglish
                //                        },
                //                        new Dictionary<VersionKeys, string> {
                //                            { VersionKeys.ProductName, "Rebuild Test Executable" },
                //                            { VersionKeys.ProductVersion, "Awesome" }
                //                        }
                //                    }
                //                }
                //    },
                //    new ManifestParameters {
                //        Stream = new FileStream("C:\\Rebuild\\main.manifest", FileMode.Open)
                //    }
                //}
            };

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
