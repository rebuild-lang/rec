using REC.Packaging.Code;
using REC.Packaging.Data;
using System.Collections.Generic;
using System;
using REC.Packaging.x86;

namespace REC.Packaging
{
    internal struct Version
    {
        public uint Major;
        public uint Minor;
    }

    internal interface IExecutable
    {
        string Name { get; }
        Version Version { get; }
        DateTime Timestamp { get; }

        IList<IInstruction> Instructions { get; }
        ILabel EntryLabel { get; }
        IList<IUninitializedDataEntry> UninitializedData { get; }
        IList<IInitializedDataEntry> InitializedData { get; }
        IList<IImportDll> DllImports { get; }
    }

    internal class Executable : IExecutable
    {
        public string Name { get; set; }
        public Version Version { get; set; }
        public DateTime Timestamp { get; set; }

        public IList<IInstruction> Instructions { get; } = new List<IInstruction>();
        public ILabel EntryLabel { get; set; }
        public IList<IUninitializedDataEntry> UninitializedData { get; } = new List<IUninitializedDataEntry>();
        public IList<IInitializedDataEntry> InitializedData { get; } = new List<IInitializedDataEntry>();
        public IList<IImportDll> DllImports { get; } = new List<IImportDll>();
    }
}
