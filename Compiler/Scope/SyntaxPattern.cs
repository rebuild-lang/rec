using System;
using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Tools;

namespace REC.Scope
{
    using SortedSyntaxPatterns = SortedSet<SyntaxPattern>;
    using NameToSortedSyntax = Dictionary<string, SyntaxPatternPool>;

    public class SyntaxPattern : IComparable<SyntaxPattern>
    {
        public int MinArgumentCount;
        public int? MaxArgumentCount; // null means possible endless arguments

        public readonly HashSet<IFunctionDeclaration> Members = new HashSet<IFunctionDeclaration>();
        public IBindingLevel BindingLevel;

        public string Name => Members.First().Name;

        public SyntaxPattern(IFunctionDeclaration declaration) {
            MinArgumentCount = declaration.MandatoryRightArgumentCount();
            MaxArgumentCount = declaration.MaxRightArgumentCount();
            Members.Add(declaration);
        }

        bool LessThan(SyntaxPattern other) {
            return MaxArgumentCount.HasValue
                && MaxArgumentCount.Value < other.MinArgumentCount;
        }

        public bool Overlaps(SyntaxPattern other) {
            return (!other.MaxArgumentCount.HasValue || MinArgumentCount <= other.MaxArgumentCount.Value) 
                && (!MaxArgumentCount.HasValue || MaxArgumentCount.Value >= other.MinArgumentCount);
        }

        public int CompareTo(SyntaxPattern other) {
            if (Overlaps(other)) return 0;
            if (LessThan(other)) return -1;
            return 1;
        }
    }

    public class SyntaxPatternPool
    {
        readonly SortedSyntaxPatterns _sorted = new SortedSyntaxPatterns();

        public SyntaxPattern Add(IFunctionDeclaration declaration) {
            var syntaxPattern = new SyntaxPattern(declaration);
            var overlap = _sorted.GetViewBetween(syntaxPattern, syntaxPattern);
            switch (overlap.Count) {
                case 0: // no overlap => just add it
                    _sorted.Add(syntaxPattern);
                    break;
                case 1: // one overlap exists => merge with new declaration
                    syntaxPattern = overlap.First();
                    MergeDeclaration(syntaxPattern, declaration);
                    break;
                default: // multiple overlaps => join them to new pattern
                    // overlap is a view - only valid when _sorted is not changed
                    foreach (var pattern in overlap) {
                        foreach (var member in pattern.Members) {
                            MergeDeclaration(syntaxPattern, member);
                        }
                    }
                    _sorted.RemoveWhere(x => x.Overlaps(syntaxPattern)); // remove existing overlaps
                    _sorted.Add(syntaxPattern); // add the new bundle
                    break;
            }
            return syntaxPattern;
        }

        private static void MergeDeclaration(SyntaxPattern syntaxPattern, IFunctionDeclaration declaration) {
            if (syntaxPattern.Members.Contains(declaration)) return;

            syntaxPattern.MinArgumentCount = Math.Min(syntaxPattern.MinArgumentCount, declaration.MandatoryRightArgumentCount());
            if (syntaxPattern.MaxArgumentCount.HasValue) {
                var max = declaration.MaxRightArgumentCount();
                syntaxPattern.MaxArgumentCount = !max.HasValue ? null : 
                    (int?) Math.Max(syntaxPattern.MaxArgumentCount.Value, max.Value);
            }
            syntaxPattern.Members.Add(declaration);
        }
    }

    public class NamedSyntaxPatterns
    {
        readonly NameToSortedSyntax _index = new NameToSortedSyntax();

        //public SyntaxPattern Find(string name, int argumentCount);
        //public SyntaxPattern Find(IFunctionDeclaration declaration);

        public SyntaxPattern Add(IFunctionDeclaration declaration) {
            var syntax = _index.GetOrAdd(
                declaration.Name,
                () => new SyntaxPatternPool());
            return syntax.Add(declaration);
        }
    }
}