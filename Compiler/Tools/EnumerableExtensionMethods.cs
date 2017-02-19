using System;
using System.Collections.Generic;
using System.Linq;

namespace REC.Tools
{
    public static class EnumerableExtensionMethods
    {
        public static IEnumerable<T> Before<T>(this IEnumerable<T> enumerable, IEnumerator<T> rest) {
            return FlattenTo<T>(new object[] {enumerable, rest});
        }

        public static IEnumerable<T> Yield<T>(this T self) {
            yield return self;
        }

        public static IEnumerable<T> YieldAll<T>(this IEnumerator<T> self) {
            while (self.MoveNext()) {
                yield return self.Current; // allow already flattened enumerators
            }
        }

        // Flatten nested enumerable to specific type
        // !! type T cannot implement IEnumerable or IEnumerator
        public static IEnumerable<T> FlattenTo<T>(this IEnumerable<dynamic> enumerable) {
            return Flattener<T>.Flatten(enumerable);
        }

        static class Flattener<T> // Flattener encapsulates Type parameter, to enable dynamic dispatch
        {
            static IEnumerable<T> Flatten(T t) {
                yield return t; // normal case
            }

            static IEnumerable<T> Flatten(object d) {
                if (d is IEnumerator<T> et) return et.YieldAll();
                if (d is IEnumerable<T> eb) return eb;
                throw new InvalidCastException();
            }

            public static IEnumerable<T> Flatten(IEnumerable<dynamic> d) {
                return d.SelectMany<dynamic, T>(i => Flatten(i)); // recursively flatten nested enumerable
            }
        }
    }
}
