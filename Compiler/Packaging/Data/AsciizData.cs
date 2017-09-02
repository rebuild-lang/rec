using System.IO;
using System.Text;

namespace REC.Packaging.Data
{
    internal interface IAsciizData : IDataEntry
    {
        byte[] Text { get; }
    }

    class AsciizData : AbstractDataEntry, IAsciizData
    {
        public byte[] Text { get; }

        public override DataEntryFlags Flags => DataEntryFlags.Fixed;

        public override void Write(BinaryWriter binaryWriter) {
            binaryWriter.Write(Text);
            binaryWriter.Write((byte)0);
        }

        public AsciizData(string text) {
            Text = Encoding.ASCII.GetBytes(text);
            Size.SetValue((ulong)Text.Length + 1);
        }
    }
}
