using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace REC.Tests.Experiments
{
    [TestFixture]
    public class DynamicTests
    {
        // This test shows how a dynamic dispatch without virtual functions works in C#
        interface IInterface
        {}

        class Derived2 : IInterface
        {}

        class Derived1 : IInterface
        {}

        static class Runner
        {
            // ReSharper disable once UnusedParameter.Local
            public static string Foo(Derived1 derived1) {
                return "Derived1";
            }

            // ReSharper disable once UnusedParameter.Local
            public static string Foo(Derived2 derived2) {
                return "Derived2";
            }
        }

        [Test]
        public void RunIt() {
            var list = new List<IInterface> {new Derived1(), new Derived2()};

            var result = list.Select(i => Runner.Foo((dynamic) i)).Cast<string>().ToList();

            Assert.AreEqual(new List<string> {"Derived1", "Derived2"}, result);
        }
    }
}
