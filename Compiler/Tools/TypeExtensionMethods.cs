using System;

namespace REC.Tools
{
    public static class TypeExtensionMethods
    {
        public static object CreateInstance(this Type type) {
            return Activator.CreateInstance(type);
        }
    }
}