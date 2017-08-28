using REC.Packaging.Image;
using REC.Packaging.x86;
using System.Collections.Generic;
using System.IO;
using System.Text;
using REC.Packaging.Code;
using System.Linq;
using System.Collections;

namespace REC.Packaging.PortableExecutable
{
    internal interface IImports : IEnumerable
    {
        IEnumerable<IImportDll> Imports { get; }
        IBindProvider<ulong> BaseAddress { get; }

        void Add(IImportDll dll);
    }

    internal interface IImportSection : ISection, IImports
    {
        IBindProvider<MagicNumber> Magic { get; }
        //IEnumerable<IImportDll> Imports { get; }

        IValueProvider<uint> ImportTableRVA { get; }
        IValueProvider<uint> ImportTableSize { get; }
        IValueProvider<uint> ImportAddressTableRVA { get; }
        IValueProvider<uint> ImportAddressTableSize { get; }

        //void AddImport(IImportDll dll);
    }

    internal class ImportSection : AbstractImagePart, IImportSection {

        public byte[] Name => Encoding.ASCII.GetBytes(".idata");
        public SectionFlags Characteristics => SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ;

        public IBindProvider<MagicNumber> Magic { get; } = new BindProvider<MagicNumber>();
        private IList<IImportDll> Imports { get; } = new List<IImportDll>();
        IEnumerable<IImportDll> IImports.Imports => Imports;

        public IBindProvider<ulong> BaseAddress { get; } = new BindProvider<ulong>();

        IBindProvider<uint> _importTableRVA = new BindProvider<uint>();
        IBindProvider<uint> _importTableSize = new BindProvider<uint>();
        IBindProvider<uint> _importAddressTableRVA = new BindProvider<uint>();
        IBindProvider<uint> _importAddressTableSize = new BindProvider<uint>();
        public IValueProvider<uint> ImportTableRVA => _importTableRVA;
        public IValueProvider<uint> ImportTableSize => _importTableSize;
        public IValueProvider<uint> ImportAddressTableRVA => _importAddressTableRVA;
        public IValueProvider<uint> ImportAddressTableSize => _importAddressTableSize;

        class DirectoryTable {
            public uint LookupTableRVA;
            public uint TimeStamp = 0;
            public uint ForwarderChain = 0;
            public uint NameRVA; // name of DLL
            public uint BindTableRVA; // 

            public static uint WriteSize = 20;

            public void Write(BinaryWriter bw) {
                bw.Write(LookupTableRVA);
                bw.Write(TimeStamp);
                bw.Write(ForwarderChain);
                bw.Write(NameRVA);
                bw.Write(BindTableRVA);
            }

            public static void WriteTerminator(BinaryWriter bw) {
                bw.Write(new byte[WriteSize]);
            }
        }
        class LookupEntry {
            public bool IsOrdinal;
            public ushort OrdinalNumber;
            public uint NameTableRVA;

            public static uint WriteSize(MagicNumber magic) => magic == MagicNumber.PE32 ? 4u : 8u;

            public void Write(BinaryWriter bw, MagicNumber magic) {
                if (magic == MagicNumber.PE32) {
                    if (IsOrdinal)
                        bw.Write(0x80000000u + OrdinalNumber);
                    else
                        bw.Write(NameTableRVA);
                }
                else {
                    if (IsOrdinal)
                        bw.Write(0x8000000000000000u + OrdinalNumber);
                    else
                        bw.Write((ulong)NameTableRVA);
                }
            }
            public static void WriteTerminator(BinaryWriter bw, MagicNumber magic) {
                if (magic == MagicNumber.PE32)
                    bw.Write((uint)0);
                else
                    bw.Write((ulong)0);
            }
        }

        static uint StringWriteSize(string str) {
            var bytes = Encoding.ASCII.GetBytes(str);
            return (uint)(bytes.Length + (bytes.Length % 2 == 0 ? 2 : 1));
        }
        static void StringWrite(BinaryWriter bw, string str) {
            var bytes = Encoding.ASCII.GetBytes(str);
            bw.Write(bytes);
            if (bytes.Length % 2 == 0) bw.Write((ushort)0); else bw.Write((byte)0);
        }

        public class HintNameEntry {
            public ushort Hint;
            public string Name;

            public uint WriteSize() {
                var bytes = Encoding.ASCII.GetBytes(Name);
                return (uint)(2 + StringWriteSize(Name));
            }

            public void Write(BinaryWriter bw) {
                bw.Write(Hint);
                StringWrite(bw, Name);
            }
        }
        public class DLL {
            public string Name;
            public HintNameEntry[] Functions;
        }

        DirectoryTable[] tables;
        LookupEntry[][] lookupEntries;
        HintNameEntry[] hintNames;
        string[] dllNames;

        private ulong? UpdateTables() {
            if (!MemoryOffset.Value.HasValue) return null;
            if (!Magic.Value.HasValue) return null;

            var imports = Imports.Select(dllImport => new ImportSection.DLL {
                Name = dllImport.Name,
                Functions = dllImport.AllEntries.Select(entry => {
                    switch (entry) {
                    case INamedImportDllEntry named:
                        return new ImportSection.HintNameEntry {
                            Hint = (ushort)named.Hint,
                            Name = named.Name
                        };
                    case INumberedImportDllEntry numbered:
                        return new ImportSection.HintNameEntry {
                            Hint = (ushort)numbered.Number
                        };
                    default:
                        throw new InvalidDataException("Wrong Entry type");
                    }
                }).ToArray()
            }).ToArray();

            tables = new DirectoryTable[Imports.Count];
            lookupEntries = new LookupEntry[Imports.Count][];
            dllNames = new string[Imports.Count];
            var _hintNames = new List<HintNameEntry>();
            // Import Lookup Tables
            var rva = (uint)MemoryOffset.Value.Value;
            var firstLookupRVA = rva + (uint)(tables.Length + 1) * DirectoryTable.WriteSize;
            var lookupRVA = firstLookupRVA;
            var lookupEntrySize = LookupEntry.WriteSize(Magic.Value.Value);
            var i = 0;
            foreach (var import in imports) {
                tables[i] = new DirectoryTable();
                lookupEntries[i] = new LookupEntry[import.Functions.Length];
                dllNames[i] = import.Name;
                tables[i].LookupTableRVA = lookupRVA;

                lookupRVA += (uint)(import.Functions.Length + 1) * lookupEntrySize; // terminator
                i++;
            }
            var lookupSize = lookupRVA - firstLookupRVA;
            _importTableRVA.SetValue(rva);
            _importTableSize.SetValue(firstLookupRVA - rva);
            // bindTables
            _importAddressTableRVA.SetValue(lookupRVA);
            _importAddressTableSize.SetValue(lookupSize);
            i = 0;
            var baseAddress = BaseAddress.Value.GetValueOrDefault(0);
            foreach (var table in tables) {
                table.BindTableRVA = table.LookupTableRVA + (lookupRVA - firstLookupRVA);
                var bindRVA = table.BindTableRVA;
                foreach (var dllEntry in Imports[i].AllEntries) {
                    dllEntry.MemoryAddress.SetValue(bindRVA + baseAddress);
                    bindRVA += lookupEntrySize;
                }
                i++;
            }
            var hintRVA = lookupRVA + lookupSize;
            i = 0;
            foreach (var import in imports) {
                var j = 0;
                foreach (var entry in import.Functions) {
                    lookupEntries[i][j] = new LookupEntry();
                    var hasName = (entry.Name.Length != 0);
                    lookupEntries[i][j].IsOrdinal = !hasName;
                    if (hasName) {
                        _hintNames.Add(entry);
                        lookupEntries[i][j].NameTableRVA = hintRVA;
                        hintRVA += entry.WriteSize();
                    }
                    else {
                        lookupEntries[i][j].OrdinalNumber = entry.Hint;
                    }
                    j++;
                }
                i++;
            }

            // dllNames
            var namesRVA = hintRVA;
            i = 0;
            foreach (var import in imports) {
                tables[i].NameRVA = namesRVA;
                namesRVA += StringWriteSize(import.Name);
                i++;
            }
            hintNames = _hintNames.ToArray();
            return namesRVA - rva;
        }

        private IBinding _sizeBinding;
        public ImportSection() {
            _sizeBinding = FuncBinding.Create(new ITriggerProvider[] { MemoryOffset, BaseAddress, Magic }, UpdateTables, Size);
        }

        public void Add(IImportDll dll) {
            Imports.Add(dll);
            _sizeBinding.Refresh();
        }

        public override void Write(BinaryWriter bw) {
            var magic = Magic.Value.Value;
            foreach (var table in tables) table.Write(bw);
            DirectoryTable.WriteTerminator(bw);

            foreach (var lookups in lookupEntries) // lookup Tables
            {
                foreach (var lookup in lookups) lookup.Write(bw, magic);
                LookupEntry.WriteTerminator(bw, magic);
            }
            foreach (var lookups in lookupEntries) // bindTables
            {
                foreach (var lookup in lookups) lookup.Write(bw, magic);
                LookupEntry.WriteTerminator(bw, magic);
            }
            foreach (var hint in hintNames) hint.Write(bw);

            foreach (var name in dllNames) {
                StringWrite(bw, name);
            }
        }

        public IEnumerator GetEnumerator() => throw new System.NotImplementedException();
    }
}
