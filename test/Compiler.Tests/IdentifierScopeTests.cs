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
            public string Label { get; set; }
            public ICallableEntry CallableEntry { get; } = null;
            public bool IsCompileTime { get; } = false;
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
            var myScope = new Scope.Scope(childScope) {new Entry { Label = "label"}};

            var testIdentifier = new Entry { Label = "label" };
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
            Assert.AreEqual(testIdentifier, childScope[testIdentifier.Label]);
            Assert.AreNotEqual(testIdentifier, myScope[testIdentifier.Label]);
        }

        [Test()]
        public void GetEnumerator() {
            var parentScope = new Scope.Scope() { new Entry { Label = "label" } };
            var childScope = new Scope.Scope(parentScope);
            var myScope = new Scope.Scope(childScope) { new Entry { Label = "fun" } };

            var result = myScope.Select(i => i.Label).ToList();

            Assert.AreEqual(new List<string>() { "fun", "label" }, result);
        }
    }
}