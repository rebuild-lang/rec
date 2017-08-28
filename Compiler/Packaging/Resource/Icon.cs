using System.IO;

namespace REC.Packaging.Resource
{
    public class IconParameters
    {
        public string Name;
        public uint Ordinal;
        public Languages Language = Languages.UsEnglish;
        public CodePages CodePage = 0;
        public Stream Stream; // .ico file format
    }

    internal class IconGroup : IEntryData
    {
        public ushort Reserved => 0;
        public ushort ResourceType => 1;
        public ushort Count => (ushort)Entries.Length;
        public ushort Padding => 0;

        public struct Entry
        {
            public byte Width;
            public byte Height;
            public byte ColorCount;
            public byte Reserved => 0;
            public ushort Planes;
            public ushort BitCount;
            public uint Size;
            public ushort Ordinal;

            public static uint WriteSize => 14;
            public void Write(BinaryWriter bw) {
                bw.Write(Width);
                bw.Write(Height);
                bw.Write(ColorCount);
                bw.Write(Reserved);
                bw.Write(Planes);
                bw.Write(BitCount);
                bw.Write(Size);
                bw.Write(Ordinal);
            }
        }
        public Entry[] Entries = new Entry[0];

        public uint WriteSize => 6 + (uint)Entries.Length * Entry.WriteSize;

        public void Write(BinaryWriter bw) {
            bw.Write(Reserved);
            bw.Write(ResourceType);
            bw.Write(Count);
            foreach (var entry in Entries) entry.Write(bw);
        }
    }
}
