using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using REC.Scope;
using REC.Instance;

namespace REC.Tests.Scope
{
    [TestFixture]
    public class IdentifierScopeTests
    {
        class TestInstance : AbstractInstance
        {
            public override string Name { get; }

            internal TestInstance(string name) {
                Name = name;
            }
        }

        [Test]
        public void Add()
        {
            var parentScope = new ParentedIdentifierScope();
            var childScope = new ParentedIdentifierScope { Parent = parentScope };
            var myScope = new ParentedIdentifierScope { Parent = childScope };

            var myEntry = new TestInstance(name: "label");
            myScope.Add(myEntry);

            var testIdentifier = new TestInstance(myEntry.Name);

            var result = parentScope.Add(testIdentifier);
            Assert.IsTrue(result);
            Assert.IsFalse(parentScope.Add(testIdentifier));

            Assert.AreEqual(testIdentifier, parentScope[testIdentifier.Name]);
            Assert.AreEqual(testIdentifier, childScope[testIdentifier.Name]);
            Assert.AreEqual(myEntry, myScope[testIdentifier.Name]);
            Assert.AreNotEqual(testIdentifier, myScope[testIdentifier.Name]);
        }

        [Test]
        public void GetEnumerator()
        {
            var parentScope = new ParentedIdentifierScope { new TestInstance(name: "label") };
            var childScope = new ParentedIdentifierScope { Parent = parentScope };
            var myScope = new ParentedIdentifierScope { Parent = childScope };
            myScope.Add(new TestInstance(name: "fun"));

            var result = myScope.Select(i => i.Name).ToList();

            Assert.AreEqual(new List<string> { "fun", "label" }, result);
        }
    }
}
