using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace REC.Packaging.x86
{

    internal enum Registers : byte
    {
        AX, CX, DX, BX, SP, BP, SI, DI,
        R0 = AX, R1 = CX, R2 = DX, R3 = CX, R4 = SP, R5 = BP, R6 = SI, R7 = DI,
        R8, R9, R10, R11, R12, R13, R14, R15, // new
        MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7, // SSE (XMMn) & AVX (YMMn)
    }
    internal enum RegisterTypes : byte
    {
        Byte, Word, DWord, QWord,
        Byte2nd, // 2nd lowest byte (only R0...R3)

        // SIMD Vectors
        Byte8, Byte16, Byte32,
        Word4, Word8, Word16, Word32,
        DWord4, DWord8, DWord16,
        QWord2, QWord4, QWord8,
        Float, Float2, Float4, Float8, Float16,
        Double, Double2, Double4, Double8,
    }
    internal enum AddressScale : byte
    {
        One = 1, Two = 2, Four = 4, Eight = 8,
    }
    internal struct Addressing
    {
        public Registers Base;
        public Registers Index;
        public AddressScale Scale;
        public ulong Offset;
    }

    internal interface IAddressConsumer
    {
        void SetAddress(ulong address);
    }
    internal interface IAddressProvider
    {
        ulong Address { get; }
        IEnumerable<IAddressConsumer> Consumer { get; }

        void AddAddressConsumer(IAddressConsumer consumer);
    }
    delegate void SizeChangedHandler(ISizeProvider provider);
    internal interface ISizeProvider
    {
        ulong Size { get; }
        event SizeChangedHandler SizeChanged;
    }

    internal interface IImportDllEntry : IAddressProvider
    {
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

        IImportDllEntry FindEntryByName(string name);
        IImportDllEntry FindEntryByHint(uint hint);
        IImportDllEntry FindEntryByNumber(uint number);

        INamedImportDllEntry AddNamed(string name, uint hint = 0);
        INumberedImportDllEntry AddNumbered(uint number);
    }

    internal interface IInstruction : IAddressProvider, ISizeProvider
    {
        void SetAddress(ulong address);
        void Write(BinaryWriter bw);
    }

    internal interface ILabel : IInstruction
    {
        string Name { get; }
    }

    internal interface ICallDllEntry : IInstruction, IAddressConsumer
    {
        IImportDllEntry DllEntry { get; }
    }

    internal interface IPushValue : IInstruction
    {
        uint Value { get; }
    }
    /*
    internal class Label : IInstruction
    {
        public uint MinSize => 0;
        public uint MaxSize => 0;

        public void Write(BinaryWriter bw)
        {
            // empty
        }
    }

    internal class Call : IInstruction
    {
        public uint MinSize => 3;
        public uint MaxSize => 6;

        public void Write(BinaryWriter bw)
        {
        }
    }

    internal class Jump : IInstruction
    {
        public uint MinSize => 2;
        public uint MaxSize => 6;

        public void Write(BinaryWriter bw)
        {
        }
    }

    internal class Push : IInstruction
    {
        public uint MinSize => throw new NotImplementedException();
        public uint MaxSize => throw new NotImplementedException();

        public void Write(BinaryWriter bw)
        {
        }
    }

    internal class Pop : IInstruction
    {
        public uint MinSize => throw new NotImplementedException();
        public uint MaxSize => throw new NotImplementedException();

        public void Write(BinaryWriter bw)
        {
        }
    }

    internal class Move : IInstruction
    {
        public uint MinSize => throw new NotImplementedException();
        public uint MaxSize => throw new NotImplementedException();

        public void Write(BinaryWriter bw)
        {
        }
    }

    internal class Add : IInstruction
    {
        public uint MinSize => throw new NotImplementedException();
        public uint MaxSize => throw new NotImplementedException();

        public void Write(BinaryWriter bw)
        {
        }
    }*/
}
