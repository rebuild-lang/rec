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
        public void IdentifierScope() {
            var parentScope = new Scope.Scope();
            var childScope = new Scope.Scope(parentScope);

            Assert.AreEqual(childScope.Parent, parentScope);
            Assert.IsNull(parentScope.Parent);

            var childScopeDisposed = false;
            childScope.Disposed += scope => {
                Assert.AreEqual(childScope, scope);
                childScopeDisposed = true;
            };
            var parentScopeDisposed = false;
            parentScope.Disposed += scope =>
            {
                // ReSharper disable once AccessToDisposedClosure
                Assert.AreEqual(parentScope, scope);
                parentScopeDisposed = true;
            };

            parentScope.Dispose();

            Assert.IsTrue(parentScopeDisposed);
            Assert.IsTrue(childScopeDisposed);
        }

        [Test()]
        public void Add() {
            var parentScope = new Scope.Scope();
            var childScope = new Scope.Scope(parentScope);
            var myScope = new Scope.Scope(childScope) {new Entry { Name = "label"}};

            var testIdentifier = new Entry { Name = "label" };
            var parentIdentifierAdded = false;
            parentScope.IdentifierAdded += (scope, identifier) =>
            {
                Assert.AreEqual(parentScope, scope);
                Assert.AreEqual(testIdentifier, identifier);
                parentIdentifierAdded = true;
            };

            var childIdentifierAdded = false;
            childScope.IdentifierAdded += (scope, identifier) => {
                Assert.AreEqual(parentScope, scope);
                Assert.AreEqual(testIdentifier, identifier);
                childIdentifierAdded = true;
            };

            const bool myIdentifierAdded = false;
            childScope.IdentifierAdded += (scope, identifier) => childIdentifierAdded = true;

            var result = parentScope.Add(testIdentifier);
            Assert.IsTrue(result);
            Assert.IsFalse(parentScope.Add(testIdentifier));

            Assert.IsTrue(parentIdentifierAdded);
            Assert.IsTrue(childIdentifierAdded);
            Assert.IsFalse(myIdentifierAdded);
            Assert.AreEqual(testIdentifier, childScope[testIdentifier.Name]);
            Assert.AreNotEqual(testIdentifier, myScope[testIdentifier.Name]);
        }

        [Test()]
        public void GetEnumerator() {
            var parentScope = new Scope.Scope() { new Entry { Name = "label" } };
            var childScope = new Scope.Scope(parentScope);
            var myScope = new Scope.Scope(childScope) { new Entry { Name = "fun" } };

            var result = myScope.Select(i => i.Name).ToList();

            Assert.AreEqual(new List<string>() { "fun", "label" }, result);
        }
    }
}