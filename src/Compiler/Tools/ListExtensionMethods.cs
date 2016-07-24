using System.Collections.Generic;

namespace REC.Tools
{
    public static class ListExtensionMethods
    {
        public static T First<T>(this IList<T> list) => list[0];
        public static T Last<T>(this IList<T> list) => list[list.Count - 1];

        public static bool IsEmpty<T>(this IReadOnlyCollection<T> list) => list.Count == 0;

        public static bool IsEmpty(this string str) => str.Length == 0;
    }
}