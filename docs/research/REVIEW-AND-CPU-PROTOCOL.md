# External review and the older CPU protocol

This note compares technical review claims with project evidence. It does not change V4.

## What the review gets right

V4 checks the trigger protocol at entry and before an automatic patch. An already-present marker produces `TOO_LATE`. It raises TPL to TPL_NOTIFY around the write/permission operations, above the warning notification's TPL_CALLBACK. Rollback depends on a fresh page walk, not a previously writable state.

V4 does not clear CR0.WP. Without EFI_MEMORY_ATTRIBUTE_PROTOCOL, it requires both a writable page-table chain and a memory descriptor without EFI_MEMORY_RO; otherwise it refuses. Zero instances of that newer protocol's GUID were found in the 318 decompressed PE files inspected. That is static absence evidence; a live protocol lookup is the direct test.

## Conclusions that need qualification

- **Phase 1 cannot establish DriverOrder timing.** It starts through the boot menu. The later automatic driver log and observed warning behavior answer that different question.
- **Marker absence is evidence, not an absolute guarantee.** It describes the queried protocol state. It does not by itself prove that the warning has never run on another boot path, nor make the entire discovery-to-write sequence atomic.
- **A V4 permission refusal would not prove that chip modification is the only alternative.** An older firmware memory-control interface exists in this image. Its usefulness remains untested.
- **The initialization gate is outside the current architecture's timing window.** It runs during DXE dispatch before an external DriverOrder helper. In the inspected module, the entry starts at RVA `0xE90`, the EC-result test is at `0xEF2`, and its branch is at `0xEF4`. The review's `0xEB0` is not a verified instruction-boundary patch address.

## Static lead: EFI_CPU_ARCH_PROTOCOL

The newer protocol GUID is `F4560CF6-40EC-4B4A-A192-BF1D57D0B189`. The older CPU Architectural Protocol uses `26BACCB1-6F42-11D4-BCE7-0080C73C8881` and includes SetMemoryAttributes. The interface definition is in the pinned [TianoCore Cpu.h](https://github.com/tianocore/edk2/blob/edk2-stable202402/MdePkg/Include/Protocol/Cpu.h).

In GKCN65WW's CpuDxe, disassembly identified the following RVAs:

| Location | Observation |
| --- | --- |
| `0x9700` | CPU Architectural Protocol GUID |
| `0x9810` | Installed interface table |
| `0x1945`–`0x195D` | Interface address, GUID and handle passed to InstallMultipleProtocolInterfaces |
| `0x10F8` | Eighth interface function, SetMemoryAttributes |
| `0x1114`, `0x112A` | State checks involving `0xA588` and `0xA480`; some states can return success without the desired change |
| `0x1139` | Protection-attribute mask `0x26000`, covering RP, XP and RO |
| `0x124D` | Call into memory-update routine `0x22A8` |
| `0x22A8` onward | Alignment checks and lower-level update path |

These are offsets inside the extracted PE, not USB-file offsets or addresses to type into a memory editor. The full supplied BIOS hash is recorded in the firmware reassessment. This analysis is an inference from static code, not a successful live API call.

Before considering an implementation, determine the live protocol's availability and behavior, how to preserve caching/read/execute attributes, the affected page granularity, and how to verify both the writable transition and restoration. A successful return code alone is insufficient. Any experiment needs new versioned files and focused failure-path tests. V4 must remain unchanged while hardware testing is pending.

The [newer protocol definition](https://github.com/tianocore/edk2/blob/edk2-stable202402/MdePkg/Include/Protocol/MemoryAttribute.h) and [UEFI driver-load ordering](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html) provide interface context; neither establishes this laptop's runtime result.
