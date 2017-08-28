using REC.Packaging.Image;
using System;
using System.IO;

namespace REC.Packaging.Code
{
    [Flags]
    enum InstructionFlags
    {
        Fixed = 1 << 0, // content does not depend on address
    }

    internal interface IInstruction : ISizeWritable
    {
        InstructionFlags Flags { get; }
        IBindProvider<ulong> MemoryAddress { get; }
        ulong? RelocationAddress { get; }
    }

    internal abstract class AbstractInstruction : AbstractSizeWritable, IInstruction
    {
        public abstract InstructionFlags Flags { get; }
        public IBindProvider<ulong> MemoryAddress { get; } = new BindProvider<ulong>();
        public virtual ulong? RelocationAddress => null;
    }
}
