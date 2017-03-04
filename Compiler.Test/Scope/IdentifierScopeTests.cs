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
        public void Add() {
            var parentScope = new ParentedIdentifierScope();
            var childScope = new ParentedIdentifierScope {Parent = parentScope};
            var myScope = new ParentedIdentifierScope {Parent = childScope};

            var myEntry = new TestInstance(name: "label");
            myScope.Add(myEntry);

            var testIdentifier = new TestInstance(myEntry.Name);

            var result = parentScope.Add(testIdentifier);
            Assert.That(result, Is.True);
            Assert.That(parentScope.Add(testIdentifier), Is.False);

            Assert.That(parentScope[testIdentifier.Name], Is.EqualTo(testIdentifier));
            Assert.That(childScope[testIdentifier.Name], Is.EqualTo(testIdentifier));
            Assert.That(myScope[testIdentifier.Name], Is.EqualTo(myEntry));
            Assert.That(myScope[testIdentifier.Name], Is.Not.EqualTo(testIdentifier));
        }

        [Test]
        public void GetEnumerator() {
            var parentScope = new ParentedIdentifierScope {new TestInstance(name: "label")};
            var childScope = new ParentedIdentifierScope {Parent = parentScope};
            var myScope = new ParentedIdentifierScope {Parent = childScope};
            myScope.Add(new TestInstance(name: "fun"));

            var result = myScope.Select(i => i.Name).ToList();

            Assert.That(result, Is.EquivalentTo(new[] {"fun", "label"}));
        }
    }
}
