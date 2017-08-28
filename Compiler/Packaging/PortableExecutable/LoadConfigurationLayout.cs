using System.IO;

namespace REC.Packaging.PortableExecutable
{
    internal class LoadConfigurationLayout
    {
        public uint Characteristics => 0;
        public uint TimeStamp;
        public ushort MajorVersion;
        public ushort MinorVersion;
        public uint GlobalFlagsClear; // The global loader flags to clear for this process as the loader starts the process.
        public uint GlobalFlagsSet; // The global loader flags to set for this process as the loader starts the process.
        public uint CriticalSectionDefaultTimeout; // The default timeout value to use for this process’s critical sections that are abandoned.
        public ulong DeCommitFreeBlockThreshold; // Memory that must be freed before it is returned to the system, in bytes.
        public ulong DeCommitTotalFreeThreshold; // Total amount of free memory, in bytes.
        public ulong LockPrefixTable; // [x86 only] The VA of a list of addresses where the LOCK prefix is used so that they can be replaced with NOP on single processor machines.
        public ulong MaximumAllocationSize; // Maximum allocation size, in bytes.
        public ulong VirtualMemoryThreshold; // Maximum virtual memory size, in bytes.
        public ulong ProcessAffinityMask; // Setting this field to a non-zero value is equivalent to calling SetProcessAffinityMask with this value during process startup (.exe only)
        public uint ProcessHeapFlags; // Process heap flags that correspond to the first argument of the HeapCreate function. These flags apply to the process heap that is created during process startup.
        public ushort CSDVersion;
        public ushort Reserved => 0;
        public ulong EditList => 0;
        public ulong SecurityCookie; // A pointer to a cookie that is used by Visual C++ or GS implementation.
        public ulong SEHandlerTable; //[x86 only] The VA of the sorted table of RVAs of each valid, unique SE handler in the image.
        public ulong SEHandlerCount; //[x86 only] The count of unique handlers in the table.
        public ulong GuardCFCheckFunctionPointer; // The VA where Control Flow Guard check-function pointer is stored.
        public ulong GuardCFDispatchFunctionPointer; // The VA where Control Flow Guard  dispatch-function pointer is stored.
        public ulong GuardCFFunctionTable; // The VA of the sorted table of RVAs of each Control Flow Guard function in the image.
        public ulong GuardCFFunctionCount; // The count of unique RVAs in the above table.
        public uint GuardFlags;
        private readonly MagicNumber magic;

        LoadConfigurationLayout(MagicNumber _magic) {
            magic = _magic;
        }

        public uint WriteSize => magic == MagicNumber.PE32 ? 92u : 148u;

        public void Write(BinaryWriter bw) {
            void WriteSizeT(ulong d)
            {
                if (magic == MagicNumber.PE32)
                    bw.Write((uint)d);
                else
                    bw.Write(d);
            }

            bw.Write(Characteristics);
            bw.Write(TimeStamp);
            bw.Write(MajorVersion);
            bw.Write(MinorVersion);
            bw.Write(GlobalFlagsClear);
            bw.Write(GlobalFlagsSet);
            bw.Write(CriticalSectionDefaultTimeout);
            WriteSizeT(DeCommitFreeBlockThreshold);
            WriteSizeT(DeCommitTotalFreeThreshold);
            WriteSizeT(LockPrefixTable);
            WriteSizeT(MaximumAllocationSize);
            WriteSizeT(VirtualMemoryThreshold);
            WriteSizeT(ProcessAffinityMask);
            bw.Write(ProcessHeapFlags);
            bw.Write(CSDVersion);
            bw.Write(Reserved);
            WriteSizeT(EditList);
            WriteSizeT(SecurityCookie);
            WriteSizeT(SEHandlerTable);
            WriteSizeT(SEHandlerCount);
            WriteSizeT(GuardCFCheckFunctionPointer);
            WriteSizeT(GuardCFDispatchFunctionPointer);
            WriteSizeT(GuardCFFunctionTable);
            WriteSizeT(GuardCFFunctionCount);
            bw.Write(GuardFlags);
        }
    }
}
