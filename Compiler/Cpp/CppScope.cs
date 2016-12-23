using System;
using System.Collections.Generic;

namespace REC.Cpp
{
    using ConceptDict = Dictionary<string, string>;
    using GenerateFunc = Func<string>;

    public interface ICppScope
    {
        ConceptDict Globals { get; }
        IIndentedTextBuilder Declaration { get; }
        IIndentedTextBuilder Runtime { get; }
        string MakeLocalName(string hint = "temp");
        void EnsureGlobal(string concept, GenerateFunc func);
        void WithDeclarationIndented(Action<ICppScope> action);
        void WithRuntimeIndented(Action<ICppScope> action);
    }

    class CppScope : ICppScope
    {
        public int LocalNameCount { get; set; }
        public Dictionary<string, string> Globals { get; set; } = new Dictionary<string, string>();
        public IIndentedTextBuilder Declaration { get; set; } = new IndentedTextBuilder();
        public IIndentedTextBuilder Runtime { get; set; } = new IndentedTextBuilder {Indentation = "  "};

        public string MakeLocalName(string hint = "temp") {
            return $"_rebuild_{hint}{LocalNameCount++}";
        }

        public void EnsureGlobal(string concept, Func<string> func) {
            if (Globals.ContainsKey(concept)) return;
            Globals[concept] = func();
        }

        public void WithDeclarationIndented(Action<ICppScope> action) {
            Declaration.WithIndent(
                subDeclaration => {
                    var sub = new CppScope {
                        Globals = Globals,
                        Declaration = subDeclaration,
                        Runtime = subDeclaration,
                        LocalNameCount = LocalNameCount
                    };
                    action(sub);
                });
        }

        public void WithRuntimeIndented(Action<ICppScope> action) {
            Runtime.WithIndent(
                subRuntime => {
                    var sub = new CppScope {
                        Globals = Globals,
                        Declaration = Declaration,
                        Runtime = subRuntime,
                        LocalNameCount = LocalNameCount
                    };
                    action(sub);
                });
        }
    }
}
