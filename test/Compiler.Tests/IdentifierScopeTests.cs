using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using REC.Scope;

namespace REC.Tests
{
    [TestFixture()]
    public class IdentifierScopeTests
    {
        private class Entry : IEntry
        {
            public string Name { get; set; }
        }


        [Test()]
        public void Add() {
            var parentScope = new IdentifierScope();
            var childScope = new IdentifierScope {Parent = parentScope};
            var myScope = new IdentifierScope {Parent = childScope};

            var myEntry = new Entry {Name = "label"};
            myScope.Add(myEntry);

            var testIdentifier = new Entry { Name = myEntry.Name };

            var result = parentScope.Add(testIdentifier);
            Assert.IsTrue(result);
            Assert.IsFalse(parentScope.Add(testIdentifier));

            Assert.AreEqual(testIdentifier, parentScope[testIdentifier.Name]);
            Assert.AreEqual(testIdentifier, childScope[testIdentifier.Name]);
            Assert.AreEqual(myEntry, myScope[testIdentifier.Name]);
            Assert.AreNotEqual(testIdentifier, myScope[testIdentifier.Name]);
        }

        [Test()]
        public void GetEnumerator() {
            var parentScope = new IdentifierScope { new Entry { Name = "label" } };
            var childScope = new IdentifierScope {Parent = parentScope};
            var myScope = new IdentifierScope {Parent = childScope};
            myScope.Add(new Entry { Name = "fun" });

            var result = myScope.Select(i => i.Name).ToList();

            Assert.AreEqual(new List<string>() { "fun", "label" }, result);
        }
    }
}