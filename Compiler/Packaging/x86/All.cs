using REC.Packaging.Code;
using System;

namespace REC.Packaging.x86
{
    [Flags]
    internal enum NativeFlags : uint
    {
        Carry = 1 << 0,
        // Unused = 1<<1,
        Parity = 1 << 2,
        // Unused = 1<<3,
        Adjust = 1 << 4,
        // Unused = 1<<5,
        Zero = 1 << 6,
        Sign = 1 << 7,
        Trap = 1 << 8,
        InterruptEnable = 1 << 9,
        Direction = 1 << 10,
        Overflow = 1 << 11,
        IoPrivilegeLevel = 1 << 12 | 1 << 13,
        NestedTask = 1 << 14,
        // Unused = 1<<15,
        // --- EFlags ---
        Resume = 1 << 16,
        Virtual8086 = 1 << 17,
        AlignmentCheck = 1 << 18,
        VirtualInterrupt = 1 << 19,
        VirtualInterruptPending = 1 << 20,
        CpuIdEnabled = 1 << 21,
        // Reserved bits 22..31
        // --- RFlags ---
        // Reserved bits 32..63
    }
    internal enum Registers : byte
    {
        AX, CX, DX, BX, SP, BP, SI, DI,
        R0 = AX, R1 = CX, R2 = DX, R3 = CX, R4 = SP, R5 = BP, R6 = SI, R7 = DI,
        R8, R9, R10, R11, R12, R13, R14, R15, // new
        MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7, // SSE (XMMn) & AVX (YMMn)
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

    //
    // planned instructions
    //
    internal enum CallJumpImmediateInstructionType
    {
        CallRelative,
        JumpRelative,
        JumpOverflow, // 70
        JumpNotOverflow,
        JumpBelow, Carry = JumpBelow, NotAboveOrEqual = JumpBelow,
        JumpNotBelow, AboveOrEqual = JumpNotBelow, NotCarry = JumpNotBelow,
        JumpEqual, Zero = JumpEqual,
        JumpNotEqual, NotZero = JumpNotEqual, // 75
        JumpNotAbove, BelowOrEqual = JumpNotAbove, // signed
        JumpAbove, NotBelowOrEqual = JumpAbove, // signed
        JumpSign,
        JumpNotSign,
        JumpParity, JumpParityEven = JumpParity, // 7A
        JumpNotParity, JumpParityOdd = JumpNotParity,
        JumpLess, JumpNotGreaterOrEqual = JumpLess,
        JumpNotLess, JumpGreaterOrEqual = JumpNotLess,
        JumpNotGreater, JumpLessOrEqual = JumpNotGreater, // signed
        JumpGreater, JumpNotLessOrEqual = JumpGreater, // 7F - signed
        JumpCxZero, // E3
    }
    internal interface ICallJumpImmediateInstruction : IInstruction
    {
        CallJumpImmediateInstructionType Type { get; }
        INativeValue Immediate { get; }
    }
}
