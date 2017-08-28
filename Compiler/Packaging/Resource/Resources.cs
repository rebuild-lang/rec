using REC.Packaging.Image;
using REC.Packaging.Tools;
using REC.Tools;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace REC.Packaging.Resource
{
    internal interface IResources : IMemoryPart, IEnumerable
    {
        IEnumerable<Entry> Entries { get; }

        void Add(IconParameters p);
        void Add(ManifestParameters p);
        void Add(VersionParameters p);
    }

    class Resources : AbstractMemoryPart, IResources
    {
        private Dictionary<Types, uint> _nextOrdinal = new Dictionary<Types, uint>();
        public uint NextOrdinal(Types type) {
            var result = _nextOrdinal.GetOrAdd(type, () => 1u);
            _nextOrdinal[type]++;
            return result;
        }

        public IList<Entry> Entries { get; } = new List<Entry>();
        IEnumerable<Entry> IResources.Entries => Entries;

        public void Add(IconParameters p) {
            using (var reader = new BinaryReader(p.Stream)) {
                var dirReserved = reader.ReadUInt16();
                var dirType = reader.ReadUInt16();
                if (dirReserved != 0 || dirType != 1) throw new ArgumentException("Not an Icon");
                var numEntries = reader.ReadUInt16();

                var groupEntries = new List<IconGroup.Entry>();

                for (var i = 0; i < numEntries; i++) {
                    var width = reader.ReadByte();
                    var height = reader.ReadByte();
                    var colorCount = reader.ReadByte();
                    var reserved = reader.ReadByte();
                    var planes = reader.ReadUInt16();
                    var bitCount = reader.ReadUInt16();
                    var size = reader.ReadUInt32();
                    var offset = reader.ReadUInt32();

                    var streamPosition = reader.BaseStream.Position;
                    reader.BaseStream.Seek(offset, SeekOrigin.Begin);
                    var data = reader.ReadBytes((int)size);
                    reader.BaseStream.Seek(streamPosition, SeekOrigin.Begin);

                    var ordinal = NextOrdinal(Types.ICON);

                    groupEntries.Add(new IconGroup.Entry {
                        Width = width,
                        Height = height,
                        ColorCount = colorCount,
                        Planes = planes,
                        BitCount = bitCount,
                        Size = size,
                        Ordinal = (ushort)ordinal,
                    });
                    Entries.Add(new Entry {
                        Type = Types.ICON,
                        Ordinal = ordinal,
                        Language = p.Language,
                        CodePage = p.CodePage,
                        Data = new PlainData { Data = data }
                    });
                }

                if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                    p.Ordinal = NextOrdinal(Types.GROUP_ICON);

                Entries.Add(new Entry {
                    Type = Types.GROUP_ICON,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new IconGroup { Entries = groupEntries.ToArray() }
                });
            }
            UpdateTables();
        }

        public void Add(ManifestParameters p) {
            if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                p.Ordinal = NextOrdinal(Types.MANIFEST);

            using (var br = new BinaryReader(p.Stream)) {
                var data = br.ReadBytes((int)br.BaseStream.Length);
                //var data = Encoding.GetEncoding((int)p.CodePage).GetBytes(p.Text);
                //var data = Encoding.ASCII.GetBytes(p.Text);
                Entries.Add(new Entry {
                    Type = Types.MANIFEST,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new PlainData { Data = data }
                });
            }
            UpdateTables();
        }

        public void Add(VersionParameters p) {
            if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                p.Ordinal = NextOrdinal(Types.VERSION);

            using (var s = new MemoryStream()) {
                using (var bw = new BinaryWriter(s)) {
                    p.Write(bw);
                }
                var data = s.ToArray();
                Entries.Add(new Entry {
                    Type = Types.VERSION,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new PlainData { Data = data }
                });
            }
            UpdateTables();
        }

        public void UpdateTables() {
            var rootTable = new DirectoryTable();
            var tables = new List<DirectoryTable> { rootTable };
            var names = new List<string>();
            var descriptions = new List<DataDescription>();
            Data = new IEntryData[Entries.Count];

            var orderedResources = Entries.OrderBy(e => e.Type)
                .ThenBy(e => string.IsNullOrEmpty(e.Name))
                .ThenBy(e => e.Name).ThenBy(e => e.Ordinal)
                .ThenBy(e => e.Language);

            var typeTables = new List<ValueTuple<DirectoryTable, IEnumerable<Entry>>>();
            rootTable.OrdinalEntries = orderedResources.GroupBy(e => e.Type, (type, typeGroup) => {
                var subTypeTableIndex = (uint)tables.Count;
                var typeTable = new DirectoryTable();
                tables.Add(typeTable);
                typeTables.Add((typeTable, typeGroup));

                return new DirectoryTable.Entry {
                    IsName = false,
                    Ordinal = (uint)type,
                    IsLeaf = false,
                    SubTableIndex = subTypeTableIndex,
                };
            }).ToArray();

            var nameTables = new List<ValueTuple<DirectoryTable, IEnumerable<Entry>>>();
            foreach (var (typeTable, typeGroup) in typeTables) {
                foreach (var entry in typeGroup.GroupBy(
                    e => (e.Name, (string.IsNullOrEmpty(e.Name) ? e.Ordinal : 0)),
                    (id, nameGroup) => {
                        var subNameTableIndex = (uint)tables.Count;
                        var nameTable = new DirectoryTable();
                        tables.Add(nameTable);
                        nameTables.Add((nameTable, nameGroup));

                        var isName = !string.IsNullOrEmpty(id.Item1);
                        var nameIndex = names.Count;
                        if (isName)
                            names.Add(id.Item1);

                        return new DirectoryTable.Entry {
                            IsName = isName,
                            NameIndex = (uint)nameIndex,
                            Ordinal = id.Item2,
                            IsLeaf = false,
                            SubTableIndex = subNameTableIndex,
                        };
                    }).GroupBy(e => e.IsName)) {
                    if (entry.Key)
                        typeTable.NameEntries = entry.ToArray();
                    else
                        typeTable.OrdinalEntries = entry.ToArray();
                }
            }

            foreach (var (nameTable, nameGroup) in nameTables) {
                nameTable.OrdinalEntries = nameGroup.Select(entry => {
                    var dataIndex = (uint)descriptions.Count;

                    descriptions.Add(new DataDescription {
                        DataIndex = dataIndex,
                        Size = entry.Data.WriteSize,
                        CodePage = (uint)entry.CodePage
                    });

                    Data[dataIndex] = entry.Data;

                    return new DirectoryTable.Entry {
                        IsName = false,
                        Ordinal = (uint)entry.Language,
                        IsLeaf = true,
                        DataIndex = dataIndex,
                    };
                }).ToArray();
            }

            DirectoryTables = tables.ToArray();
            DirectoryStrings = names.ToArray();
            DataDescriptions = descriptions.ToArray();
            Size.SetValue(WriteSize);
        }

        internal class DirectoryTable
        {
            public uint Characteristics => 0;
            public uint TimeStamp;
            public ushort MajorVersion = 0;
            public ushort MinorVersion = 0;
            public ushort NumberOfNameEntries => (ushort)NameEntries.Length;
            public ushort NumberOfOrdinalEntries => (ushort)OrdinalEntries.Length;

            internal class Entry
            {
                public bool IsName;
                public uint NameIndex;
                public uint Ordinal;

                public bool IsLeaf;
                public uint DataIndex;
                public uint SubTableIndex;

                public static uint WriteSize => 8;
                public void Write(BinaryWriter bw, Func<uint, uint> nameOffset, Func<uint, uint> descriptionOffset, Func<uint, uint> tableOffset) {
                    if (IsName)
                        bw.Write(0x8000_0000u | nameOffset(NameIndex)); // actually an offset in Section
                    else
                        bw.Write(Ordinal);

                    if (IsLeaf)
                        bw.Write(descriptionOffset(DataIndex)); // actually an offset in Section
                    else
                        bw.Write(0x8000_0000u | tableOffset(SubTableIndex)); // actually an offset in Section
                }
            }
            public Entry[] NameEntries = new Entry[0];
            public Entry[] OrdinalEntries = new Entry[0];

            public uint WriteSize => 16u + (uint)(NameEntries.Length + OrdinalEntries.Length) * Entry.WriteSize;

            public void Write(BinaryWriter bw, Func<uint, uint> nameOffset, Func<uint, uint> descriptionOffset, Func<uint, uint> tableOffset) {
                bw.Write(Characteristics);
                bw.Write(TimeStamp);
                bw.Write(MajorVersion);
                bw.Write(MinorVersion);
                bw.Write(NumberOfNameEntries);
                bw.Write(NumberOfOrdinalEntries);
                foreach (var e in NameEntries) e.Write(bw, nameOffset, descriptionOffset, tableOffset);
                foreach (var e in OrdinalEntries) e.Write(bw, nameOffset, descriptionOffset, tableOffset);
            }
        }

        static uint StringWriteSize(string str) {
            return 2u + (uint)Encoding.Unicode.GetByteCount(str);
        }

        static void StringWrite(BinaryWriter bw, string str) {
            var bytes = Encoding.Unicode.GetBytes(str);
            bw.Write((ushort)str.Length);
            bw.Write(bytes);
        }

        internal class DataDescription
        {
            public uint DataIndex;
            public uint Size;
            public uint CodePage;
            public uint Reserved => 0;

            public static uint WriteSize => 16;
            public void Write(BinaryWriter bw, Func<uint, uint> indexToRVA) {
                bw.Write(indexToRVA(DataIndex));
                bw.Write(Size);
                bw.Write(CodePage);
                bw.Write(Reserved);
            }
        }

        // 3 Nested Tables: Type, Name, Language
        private DirectoryTable[] DirectoryTables;
        private DataDescription[] DataDescriptions;
        private string[] DirectoryStrings;
        private IEntryData[] Data;

        private uint TablesSize => (uint)(DirectoryTables.Sum(dt => dt.WriteSize));
        private uint DescriptionSize => (uint)(DataDescriptions.Length * DataDescription.WriteSize);
        private uint RawStringSize => (uint)DirectoryStrings.Sum(ds => StringWriteSize(ds));
        private uint StringsSize => RawStringSize.AlignTo(16);
        private uint DataSize => (uint)(Data.Sum(dt => dt.WriteSize.AlignTo(16)));

        public uint WriteSize => TablesSize + DescriptionSize + StringsSize + DataSize;

        public override void Write(BinaryWriter bw) {
            if (DirectoryTables == null) return;
            ulong baseRVA = MemoryOffset.Value.Value;
            var tableOffset = 0u;
            var descriptionOffset = tableOffset + TablesSize;
            var stringOffset = descriptionOffset + DescriptionSize;
            var dataRVA = (uint)(baseRVA + stringOffset + StringsSize);
            foreach (var table in DirectoryTables)
                table.Write(bw,
                    (index) => stringOffset + (uint)DirectoryStrings.Take((int)index).Sum(ds => StringWriteSize(ds)),
                    (index) => descriptionOffset + index * DataDescription.WriteSize,
                    (index) => tableOffset + (uint)DirectoryTables.Take((int)index).Sum(dt => dt.WriteSize));

            foreach (var dataDescription in DataDescriptions)
                dataDescription.Write(bw,
                    (index) => dataRVA + (uint)Data.Take((int)index).Sum(d => d.WriteSize.AlignTo(16)));

            foreach (var str in DirectoryStrings) StringWrite(bw, str);
            bw.PadPosition(16);

            foreach (var data in Data) {
                data.Write(bw);
                bw.PadPosition(16);
            }
        }

        public IEnumerator GetEnumerator() => throw new NotImplementedException();
    }
}
