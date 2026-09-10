# V4 validation — September 10, 2026

**Historical V4 development record: built and tested in host simulation and UEFI emulation. Not tested on the physical Lenovo.**

This record describes validation before public packaging. The single reviewed fixture is included in the public repository for host tests, but omitted from the USB download. Public CI runs the host cases; it does not reproduce this historical emulator run.

## Completed

| Check | Result |
|---|---|
| Input identity | Analyzed GKCN65WW image and extracted warning module hashes match the reassessment. Original BIOS file left unchanged. |
| Source comparison | Actual recovered August V3 source and documentation inspected; changes recorded in `COMPARISON-WITH-V3.md`. |
| Build | Zig 0.13.0 / its bundled Clang and linker; all three builds pass with warnings treated as errors. |
| Repeatability | A rebuild produced byte-identical normalized EFI binaries. |
| PE format | x64 PE32+, subsystem 10 for diagnostic/manual applications and 11 for the automatic driver. Relocation directory present; no OS imports; unsigned. |
| ABI | Compile-time checks cover the four-byte device-path header and important UEFI structure offsets. |
| Host fault injection | **33 cases passed** under AddressSanitizer and UndefinedBehaviorSanitizer. Detailed output in `HOST-TEST-RESULTS.txt`. |
| Ordinary UEFI loading | Official TianoCore Shell loaded the automatic driver successfully on OVMF with no target; it returned normally. The diagnostic opened and wrote its no-target log. |
| Emulator target | An inert copy of the supplied module was exposed as a synthetic Loaded Image. Its original firmware code was never executed. The diagnostic recognized it without patching. |
| Actual shipped driver | OVMF LoadImage/StartImage loaded the shipped Driver.efi; it changed the expected byte, returned EFI_SUCCESS and wrote a successful driver log. |
| Timing and identity refusal | An installed marker caused automatic refusal; a change outside the 16-byte signature caused whole-text identity refusal. |
| Shell installation/removal | No-argument script displayed status only. Explicit INSTALL added the named DriverOrder entry; removal of that known VM entry left the list empty. |
| Final emulator run | QEMU 8.2.2 with Ubuntu OVMF 2024.02, isolated FAT32 disk image, exit 0 and `GUEST_FAILURES=0`. Transcript in `UEFI-EMULATION-RESULTS.txt`. |
| Instruction inspection | No writes to CR0/CR3/CR4, I/O-port output or WRMSR instructions found in the shipped helpers' disassembled code sections. This supplements source review; it is not a formal proof. |
| Package layout | Correct application/driver copied into each stage; phase 1/2 BOOTX64 copies match their named applications. Shell binary and license included. |

Host cases include no target, duplicate handles, missing/wrong GUID, malformed paths, bad PE identity, changed code outside the signature, already-patched code, late/unknown trigger state, read-only pages without a protocol, failed attribute queries, missing restoration methods, clear failures, failed post-clear queries, restoration failures, current-permission-only rollback, failed readback, NX pages, unsupported protection keys, nonpresent mappings, and 4/5-level paging with 4 KiB/2 MiB/1 GiB leaf pages.

## Limits that matter

The OVMF build did **not** expose EFI_MEMORY_ATTRIBUTE_PROTOCOL. Its native RO-transition test was explicitly skipped. Clear/restore behavior and failure paths passed injected host tests, but a real firmware protocol transition must still be proven by the laptop's Phase 2 before automatic installation. The guest harness reports that skip rather than silently calling it a pass.

An earlier run using QEMU's host-directory FAT backend aborted while appending test logs. The final run used a standalone FAT32 disk image and completed normally, including the log writes. No change to the shipped helper was required for that infrastructure change; the aborted run is not counted as successful validation.

Emulation does not establish Lenovo-specific module availability, USB discovery during DriverOrder, the physical firmware's alternate BDS paths, warning suppression, or the effect of skipping the callback's USB connection call. It also does not test Windows, encryption recovery, Secure Boot signing/enrollment, or internal-disk deployment. Physical diagnostic, manual, automatic and A/B stages remain necessary.

If RO restoration fails, the helper reports an error and attempts rollback only when the **current** mapping permits it. A byte or permission change may remain until reboot if firmware restoration fails; the package does not claim infallible rollback. Use a full power-off and the USB-removal recovery path.

## Included Shell provenance

Official binary: [TianoCore edk2-archive x64 Shell.efi](https://github.com/tianocore/edk2-archive/blob/master/ShellBinPkg/UefiShell/X64/Shell.efi), downloaded September 10, 2026; 908,192 bytes.

SHA-256:

```text
e6c8c8b6ecf594927724894a394f12c1cb110faa393d6c0e01c1784f9df7cd7d
```

The accompanying [ShellBinPkg license](https://github.com/tianocore/edk2-archive/blob/master/ShellBinPkg/License.txt) is reproduced in the Phase 3 folder. The Shell is used only for inspecting, adding and removing the standard driver load option.

`BINARY-VALIDATION.json` records the three helper hashes and format checks. The public download has its own `SHA256SUMS.txt` covering its selected files, excluding the manifest itself. Its archive checksum differs from the original private package.
