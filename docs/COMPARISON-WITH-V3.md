# Comparison with the recovered August V3 package

The archived V3 package dated August 13, 2026 was inspected, including source, headers, binaries and documentation. Earlier V2 documentation and the original helper source were also compared. No confirmed physical-laptop success is recorded; proposed instructions are not treated as completed tests.

## What remains the same

The underlying patch was already present in V3. It matches the callback at RVA `0x1030` and replaces its first byte `48` with `C3` (`RET`). The expected first 16 bytes are:

```text
48 83 EC 38 E8 07 27 00 00 48 8B 0D 20 42 00 00
```

Both versions aim to modify only the loaded RAM copy for that boot. V3 already excluded its own image from scanning, inspected page permissions, avoided changing CR0.WP, provided a manual test, and used a silent DriverOrder helper with a best-effort USB log. Those ideas are not new V4 discoveries.

## Source issues found and corrected

| Area | Recovered V3 implementation | V4 implementation |
|---|---|---|
| Device-path ABI | `EFI_DEVICE_PATH_PROTOCOL.Type` was declared as UINT32. The actual UEFI field is UINT8; the whole header is four bytes. This could prevent correct firmware GUID parsing. | Correct four-byte header, compile-time ABI assertions, bounded parsing and memory-range checks. |
| Firmware identity | Broad 16-byte search across loaded images; expected RVA required. A mismatching firmware GUID rejected a target only if V3 managed to parse a GUID. | Mandatory exact firmware-file GUID, exactly one matching module handle, expected PE layout/RVA, original callback bytes and CRC32 of the **whole immutable .text section**. No fallback if the GUID is unavailable. |
| Failed attribute query | The code could accept a writable page after GetMemoryAttributes failed. It also contained a fallback attempting ClearMemoryAttributes after a failed query. This was more permissive than the technical notes' description. | A present protocol whose query fails causes refusal. No inferred original attributes and no speculative clear. |
| CR0.WP already off | V3 could treat an RO page as writable because supervisor write protection was naturally off. | Requires the actual page-table RW chain. CR0.WP is read but never changed or relied on to bypass RO. |
| Restoration prerequisites | Clear could be attempted before all restoration prerequisites were established. Some error exits attempted restoration without checking the result. | Requires both methods in advance, compares queried attributes before/after, re-walks current page tables and explicitly reports restoration failure. |
| Rollback | A failure branch could justify a rollback store using the **old** pre-change writable state even after an RO restoration attempt. | Uses only a new page walk immediately before any rollback store; never writes through a currently protected mapping. |
| Memory inspection | Scanned whole loaded images and dereferenced page-table pages without the new complete range checks. | Scans the exact candidate code section after memory-map checks; rejects unsupported mappings, protection-key configurations, user mappings and non-executable targets. |
| Timing evidence | The notes left proprietary Lenovo BDS ordering as the decisive unknown. | New static BdsDxe trace places driver loading before the identified warning trigger. Runtime logs check the marker at entry and immediately before patching; an already-present marker blocks an automatic patch. |
| Packaging | Phase 3 required separately downloading a Shell; a future silent final driver was included. | Shell and its license are bundled. Package includes only diagnostic/manual/USB-test stages, with source, tests, logs and exact-entry removal instructions. |

The whole-text CRC is a compatibility fingerprint for the inspected module, not a cryptographic trust mechanism or a substitute for Secure Boot.

The device-path layout is defined in [TianoCore's DevicePath.h](https://github.com/tianocore/edk2/blob/edk2-stable202402/MdePkg/Include/Protocol/DevicePath.h). The memory-attribute methods and their error semantics are defined in [MemoryAttribute.h](https://github.com/tianocore/edk2/blob/edk2-stable202402/MdePkg/Include/Protocol/MemoryAttribute.h).

## What the fresh BIOS analysis adds

The input `the analyzed BIOS image` is GKCN65WW, SHA-256:

```text
2b872ef768d682ae933ee7f44a4d2515935d0a49b956ff9bb4cf7fc7f6b791a6
```

The warning module is `AdapterUnsupportWarningPage`, firmware file GUID `0221D385-497A-4BBF-924E-7BDFC4C1EAD8`. Its extracted PE SHA-256 is:

```text
6ea392a3d33a65e326a58e88025aec186daffc71ea7811edd5fce2ca68c00e9f
```

Its `.text` begins at RVA `0x2A0`, virtual size `0x40BA`, CRC32 `ED5F56DD`. Both base relocations are in `.data` (`0x4668`, `0x4670`), so relocation does not invalidate that code-section fingerprint. Loaded mutable `.data` is not compared.

The BdsDxe main path obtains the DriverOrder list at `0x30C4` and executes entries at `0x30EF`. Later it enters the platform routine at `0x31A4`; the normal path reaches `0x279BC`, installing marker protocol `3725E246-7CAB-4241-95A3-A560BDF44A6C` at `0x27A02`. That is the marker registered for the warning callback. This is static evidence for a viable timing window, not a physical boot result. The warning notification is registered at TPL_CALLBACK (8); the patch briefly raises TPL_NOTIFY (16) around permission transitions and the byte operation.

The standard also defines driver load options before boot load options, but proprietary paths and USB availability still need testing. [UEFI Boot Manager specification](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html).

No simple warning-disable NVRAM preference was established. `IllegalAdapter` is status written by the warning driver, not an identified disable flag. Quiet Boot, Fast Boot, Instant Boot and charger-mode settings do not establish a safe warning-off switch. V4 therefore does not write any of them. The normal DriverOrder entry is a loader configuration, not a newly discovered hidden warning setting.

The full earlier reassessment is bundled as `FIRMWARE-REASSESSMENT.md`; read it for the callback, EC flag, IFR and BDS details. V4 does not change charger identification or power limits. Skipping the callback also skips its USB-controller connection call, which is why the walkthrough includes checking ordinary USB operation.

## What remains unproven

The laptop must establish that its live module exposes the expected GUID, its permissions permit the tested route, its firmware loads the USB driver early enough, and the warning disappears without disrupting USB behavior. Removing/reinserting the USB should reverse/reapply the result across fresh boots.

These binaries are unsigned. Temporarily disabling Secure Boot does not enroll trust for later boots. Keeping a helper running after re-enabling Secure Boot requires a separately supported trust setup. No key enrollment or internal-ESP deployment is performed by this package. [Microsoft Secure Boot overview](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).
