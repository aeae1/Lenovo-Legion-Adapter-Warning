# Phase 2 physical manual RAM test — reviewed September 10, 2026

**Outcome: the intended one-byte RAM change and readback verification succeeded. No rollback or memory-permission transition was needed.**

Evidence: the V4 manual helper screen followed by the complete BEGIN/END manual log supplied for review. This public record omits the image, device clock, and live addresses. The review date is not a claim that the firmware clock was calibrated.

| Item | Observed result |
| --- | --- |
| Mode / result / status | MANUAL / READY_OR_PATCHED / EFI_SUCCESS |
| Target | One matching module; text CRC32 ED5F56DD; image size 0x5B40; target RVA 0x1030 |
| Candidate / write / verification | candidate_ready=1; patched=1; verified=1 |
| Rollback | rollback=0 |
| Mapping | Valid identity-mapped, executable supervisor page; writable 2 MiB leaf |
| Memory type | EfiBootServicesCode (3) |
| CPU write protection | Enabled in both recorded register snapshots |
| Attribute protocol | Absent; EFI_NOT_FOUND |
| Permission transition | clear_attempted=0; cleared_ro=0; restored_ro=0 |
| Trigger | Present at entry and immediately before the manual patch |
| USB-log write | The preceding screen reported EFI_SUCCESS; the full record was subsequently supplied |

The successful byte readback verifies the expected replacement byte and unchanged remaining bytes in the local signature, as implemented by V4. It is not a complete memory-integrity proof. The target identity was checked before writing.

No permission-changing interface was called. Zero clear/restore flags are expected here, and EFI_NOT_READY clear/restore statuses mean those operations were not attempted. The read-only transition/restore path was not exercised on this laptop. The second recorded page walk matches the first; in this writable path it is the recheck immediately before the byte store, not a separate post-store page walk.

Both trigger statuses are EFI_SUCCESS. Manual mode deliberately allows a late marker to test write mechanics. Automatic mode requires the marker to be absent; this result does not establish that timing, nor that the warning was displayed or suppressed.

The result supports proceeding to the existing Phase 3 USB test after a full power-off and confirmation of normal Windows startup with the USB removed. That normal-boot observation, actual power source, attached peripherals, prior-driver-entry state, and a separately measured USB executable hash have not been explicitly reported. These details must not be inferred from the nearby power-station display or from the firmware clock.

Remaining evidence: automatic helper loading/timing, actual USB-C warning behavior, ordinary USB peripheral behavior, and removal/A-B behavior. See [issue #2](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/2) for the completed manual result and [issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) for the next stage. The earlier diagnostic report remains a historical record of Phase 1's scope.
