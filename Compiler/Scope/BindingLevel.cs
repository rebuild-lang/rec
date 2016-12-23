using System;
using System.Collections.Generic;
using REC.AST;
using REC.Tools;

namespace REC.Scope
{
    public enum Associativity
    {
        None, // <  <  No same binding strength allowed
        Left, // <= <  a+b+c evaluates (a+b)+c
        Right, // <  <= a+b+c evaluates a+(b+c)
        Both // <= <= does not matter (might give optimizations)
    }

    public enum AddSuccess
    {
        Success, // relation was added
        Redundant, // relation was already known
        ConflictingWeaker, // 
        ConflictingStronger
    }

    public interface IBindingLevel : IComparable<IBindingLevel>
    {
        Associativity Associaticity { get; }

        void ClearRelations();

        AddSuccess AddWeakerThanRelation(IBindingLevel stronger);

        AddSuccess AddStrongerThanRelation(IBindingLevel weaker);

        AddSuccess AddSameRelation(IBindingLevel same);
    }

    public static class BindingLevelFactory
    {
        static IBindingLevel Create(IFunctionDeclaration declaration, Associativity associativity) {
            var result = new BindingLevel {Associaticity = associativity};
            // add syntax pattern
            return result;
        }
    }

    class BindingLevel : IBindingLevel
    {
        public readonly HashSet<SyntaxPattern> Members = new HashSet<SyntaxPattern>();
        BindingLevelGroup Group;
        public Associativity Associaticity { get; set; }

        public int CompareTo(IBindingLevel iother) {
            var other = (BindingLevel) iother;
            if (Group == other.Group) return 0;
            if (Group == null) return -1;
            if (other.Group == null) return 1;
            return Group.CompareTo(other.Group);
        }

        public void ClearRelations() {
            if (null == Group) return;
            Group.RemoveLevel(this);
            Group = null;
        }

        public AddSuccess AddWeakerThanRelation(IBindingLevel istronger) {
            var stronger = (BindingLevel) istronger;
            return EnsureGroup().AddStronger(stronger.EnsureGroup());
        }

        public AddSuccess AddStrongerThanRelation(IBindingLevel weaker) {
            var res = weaker.AddWeakerThanRelation(this);
            switch (res) {
                case AddSuccess.ConflictingWeaker:
                    return AddSuccess.ConflictingStronger;
                case AddSuccess.ConflictingStronger:
                    return AddSuccess.ConflictingWeaker;
                default:
                    return res;
            }
        }

        public AddSuccess AddSameRelation(IBindingLevel isame) {
            var same = (BindingLevel) isame;
            var sameGroup = same.EnsureGroup();
            var res = EnsureGroup().AddSame(sameGroup);
            if (res == AddSuccess.Success) {
                foreach (var level in sameGroup.Levels) {
                    level.Group = Group;
                    Group.AddLevel(level);
                }
                sameGroup.Clear();
            }
            return res;
        }

        BindingLevelGroup EnsureGroup() {
            if (null == Group) {
                Group = new BindingLevelGroup();
                Group.AddLevel(this);
            }
            return Group;
        }
    }

    class BindingLevelGroup : IComparable<BindingLevelGroup>
    {
        readonly HashSet<BindingLevel> _levels = new HashSet<BindingLevel>();
        readonly HashSet<BindingLevelGroup> _stronger = new HashSet<BindingLevelGroup>();
        readonly HashSet<BindingLevelGroup> _weaker = new HashSet<BindingLevelGroup>();
        public IEnumerable<BindingLevel> Levels => _levels;

        public int CompareTo(BindingLevelGroup other) {
            if (_stronger.Contains(other)) return -1;
            if (_weaker.Contains(other)) return 1;
            return 0;
        }

        public void Clear() {
            _levels.Clear();
            foreach (var stronger in _stronger) stronger._weaker.Remove(this);
            foreach (var weaker in _weaker) weaker._stronger.Remove(this);
        }

        public void AddLevel(BindingLevel level) {
            _levels.Add(level);
        }

        public void RemoveLevel(BindingLevel level) {
            _levels.Remove(level);
            if (_levels.IsEmpty()) Clear();
        }

        public AddSuccess AddStronger(BindingLevelGroup strongerLevel) {
            if (_stronger.Contains(strongerLevel)) return AddSuccess.Redundant;
            if (_weaker.Contains(strongerLevel)) return AddSuccess.ConflictingWeaker;

            _stronger.Add(strongerLevel);
            _stronger.UnionWith(strongerLevel._stronger);
            strongerLevel._weaker.Add(this);
            strongerLevel._weaker.UnionWith(_weaker);

            foreach (var moreStronger in strongerLevel._stronger) {
                moreStronger._weaker.Add(this);
                moreStronger._weaker.UnionWith(_weaker);
            }

            foreach (var weaker in _weaker) {
                weaker._stronger.Add(strongerLevel);
                weaker._stronger.UnionWith(strongerLevel._stronger);
            }

            return AddSuccess.Success;
        }

        public AddSuccess AddSame(BindingLevelGroup sameLevel) {
            if (_stronger.Contains(sameLevel)) return AddSuccess.ConflictingStronger;
            if (_stronger.Overlaps(sameLevel._weaker)) return AddSuccess.ConflictingStronger;
            if (_weaker.Contains(sameLevel)) return AddSuccess.ConflictingWeaker;
            if (_weaker.Overlaps(sameLevel._stronger)) return AddSuccess.ConflictingWeaker;

            foreach (var stronger in _stronger) stronger._weaker.UnionWith(sameLevel._weaker);
            foreach (var weaker in _weaker) weaker._stronger.UnionWith(sameLevel._stronger);

            foreach (var stronger in sameLevel._stronger) {
                stronger._weaker.Add(this);
                stronger._weaker.UnionWith(_weaker);
            }
            foreach (var weaker in sameLevel._weaker) {
                weaker._stronger.Add(this);
                weaker._stronger.UnionWith(_stronger);
            }

            _stronger.UnionWith(sameLevel._stronger);
            _weaker.UnionWith(sameLevel._weaker);

            return AddSuccess.Success;
        }
    }
}
