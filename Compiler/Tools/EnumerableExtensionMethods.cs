using System.Collections.Generic;
using System.Linq;

namespace REC.Tools
{
    public static class EnumerableExtensionMethods
    {
        public static IEnumerable<T> Yield<T>(this T self) {
            yield return self;
        }

        // Flatten nested enumerable to specific type
        // !! type T cannot implement IEnumerable or IEnumerator
        public static IEnumerable<T> FlattenTo<T>(this IEnumerable<dynamic> enumerable) {
            return Flattener<T>.Flatten(enumerable);
        }

        public static IEnumerator<T> GetFlatEnumerator<T>(this IEnumerable<dynamic> enumerable) {
            return FlattenTo<T>(enumerable).GetEnumerator();
        }

        static class Flattener<T> // Flattener encapsulates Type parameter, to enable dynamic dispatch
        {
            static IEnumerable<T> Flatten(T t) {
                yield return t; // normal case
            }

            static IEnumerable<T> Flatten(IEnumerator<T> d) {
                while (d.MoveNext()) {
                    yield return d.Current; // allow already flattened enumerators
                }
            }

            public static IEnumerable<T> Flatten(IEnumerable<dynamic> d) {
                return d.SelectMany<dynamic, T>(i => Flatten(i)); // recursively flatten nested enumerable
            }
        }
    }
}
