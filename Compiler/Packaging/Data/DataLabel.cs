using System.IO;

namespace REC.Packaging.Data
{
    internal interface IDataLabel : IDataEntry
    {
        string Name { get; }
    }

    internal class DataLabel : AbstractDataEntry, IDataLabel
    {
        public string Name { get; set; }

        public override DataEntryFlags Flags => DataEntryFlags.Fixed;

        public override void Write(BinaryWriter binaryWriter) { }

        public DataLabel() => Size.SetValue(0);
    }
}
