using REC.Packaging.Code;
using REC.Tools;
using System;
using System.Collections.Generic;

namespace REC.Packaging.x86
{
    internal interface IImportDllEntry : IAddressProvider
    {
        void SetAddress(ulong address);
    }
    internal interface INamedImportDllEntry : IImportDllEntry
    {
        string Name { get; }
        uint Hint { get; } // speeds up lookup
    }
    internal interface INumberedImportDllEntry : IImportDllEntry
    {
        uint Number { get; }
    }

    internal interface IImportDll
    {
        string Name { get; }
        IEnumerable<IImportDllEntry> AllEntries { get; }

        INamedImportDllEntry AddNamed(string name, uint hint = 0);
        INumberedImportDllEntry AddNumbered(uint number);
    }
    internal interface ISearchableImportDll : IImportDll
    {
        IImportDllEntry FindEntryByName(string name);
        IImportDllEntry FindEntryByHint(uint hint);
        IImportDllEntry FindEntryByNumber(uint number);
    }

    internal class ImportDll : ISearchableImportDll
    {
        private IList<IImportDllEntry> _entries = new List<IImportDllEntry>();
        private IDictionary<string, INamedImportDllEntry> _names = new Dictionary<string, INamedImportDllEntry>();
        private IDictionary<uint, INamedImportDllEntry> _hints = new Dictionary<uint, INamedImportDllEntry>();
        private IDictionary<uint, INumberedImportDllEntry> _numbers = new Dictionary<uint, INumberedImportDllEntry>();

        private class NamedEntry : AbstractAddressProvider, INamedImportDllEntry
        {
            public string Name { get; set; }
            public uint Hint { get; set; }
            public void SetAddress(ulong address) { Address = address; }
        }
        private class NumberedEntry : AbstractAddressProvider, INumberedImportDllEntry
        {
            public uint Number { get; set; }
            public void SetAddress(ulong address) { Address = address; }
        }

        public string Name { get; set; }

        public IEnumerable<IImportDllEntry> AllEntries => _entries;

        public INamedImportDllEntry AddNamed(string name, uint hint = 0) {
            var entry = new NamedEntry { Name = name, Hint = hint };
            _entries.Add(entry);
            _names.Add(entry.Name, entry);
            if (hint != 0) _hints.Add(entry.Hint, entry);
            return entry;
        }

        public INumberedImportDllEntry AddNumbered(uint number) {
            var entry = new NumberedEntry { Number = number };
            _entries.Add(entry);
            _numbers.Add(entry.Number, entry);
            return entry;
        }

        public IImportDllEntry FindEntryByHint(uint hint) {
            return _hints.Fetch(hint, null);
        }

        public IImportDllEntry FindEntryByName(string name) {
            return _names.Fetch(name, null);
        }

        public IImportDllEntry FindEntryByNumber(uint number) {
            return _numbers.Fetch(number, null);
        }
    }
}
