using System.Collections.Generic;

namespace REC.Tools
{
    public static class ListExtensionMethods
    {
        public static T First<T>(this IList<T> list) => list[0];
        public static T Last<T>(this IList<T> list) => list[list.Count - 1];

        public static bool IsEmpty<T>(this IReadOnlyCollection<T> collection) => collection.Count == 0;
        public static bool IsEmpty<T>(this ICollection<T> collection) => collection.Count == 0;
        // Dictionary is IReadOnlyCollection and ICollection so above becomes ambiguous
        public static bool IsEmpty<TK,T>(this Dictionary<TK,T> dict) => dict.Count == 0;

        public static bool IsEmpty(this string str) => str.Length == 0;
    }
}