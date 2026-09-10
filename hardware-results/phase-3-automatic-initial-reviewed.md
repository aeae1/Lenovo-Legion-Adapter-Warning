# First automatic-driver run — reviewed September 10, 2026

**Outcome: automatic-mode execution, absent-trigger checks, and the expected one-byte RAM change/readback succeeded. Actual warning suppression has not yet been reported.**

Evidence: a screenshot of USB DriverOrder installation, followed by a complete AUTOMATIC_DRIVER log supplied after the reset/normal-startup instructions. This public summary omits photographs, device clock values, the partition GUID, and live memory/device-path addresses.

## Installation observation

The no-argument installer reported no existing driver options. Explicit INSTALL then created one entry described as `Lenovo USB-C Warning Patch v4 TEST`, pointing to the helper on a USB device path, and returned to the Shell without an error visible on screen. The observed option position was 00 and variable was Driver0000. These are historical values; removal must use a fresh list and the current position of the exact entry.

## Automatic log

| Item | Observed result |
| --- | --- |
| Mode / result / status | AUTOMATIC_DRIVER / READY_OR_PATCHED / EFI_SUCCESS |
| Target | One matching module; CRC32 ED5F56DD; image size 0x5B40; target RVA 0x1030 |
| Trigger at entry | EFI_NOT_FOUND: marker absent |
| Trigger before patch | EFI_NOT_FOUND: marker absent |
| Candidate / write / verification | candidate_ready=1; patched=1; verified=1 |
| Rollback | rollback=0 |
| Mapping | Valid, identity-mapped, executable supervisor mapping; writable 2 MiB leaf |
| Attribute protocol | Absent; EFI_NOT_FOUND |
| Permission transition | clear_attempted=0; cleared_ro=0; restored_ro=0 |
| CPU write protection | Enabled in both recorded snapshots |

Unlike the earlier manual/diagnostic observations, both automatic trigger queries found the marker absent. The helper then reported a verified byte change. This supports the expected early window in the observed run. It does not prove the marker stayed absent for every instruction between the query and store, cover all proprietary boot paths, or establish that the warning was suppressed.

No permission-control fallback was needed. Clear/restore status fields remain initialized EFI_NOT_READY because those operations were not attempted. The second recorded page walk matches the first and is the pre-store recheck in the writable branch, not a separate post-store page walk.

The sequence is consistent with the requested DriverOrder startup test after registration. The supplied log itself identifies automatic helper mode, not the mechanism that invoked it or the later Windows startup outcome. Actual power source, explicit normal Windows startup, adapter-message behavior, and USB peripheral behavior have not yet been reported. Do not infer those from the firmware clock or nearby equipment in photographs.

## Remaining evidence

Test a normal full startup with the same USB-C charger/cable that reliably produces the warning, retain the helper USB in the same port, and review a fresh driver log. Record visible warning behavior, Windows startup, and ordinary USB peripheral behavior. If suppression works, perform the planned powered-off comparison with the helper USB absent and then reinserted. A missing-media driver option can be cleaned up by firmware; inspect current registration if reinsertion does not resume the helper.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open. Internal deployment, Secure Boot trust, and other firmware versions are outside the tested scope. No source, helper binary, or release asset changed for this report.
