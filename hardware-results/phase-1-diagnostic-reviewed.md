# Phase 1 physical diagnostic — reviewed September 10, 2026

**Outcome: V4 recognized the expected firmware target and passed the read-only candidate checks. Proceed to the existing manual RAM mechanics test; no physical patch result is established yet.**

Evidence: a V4 diagnostic screen photograph followed by the complete BEGIN/END diagnostic record supplied for review. This public summary omits the photograph, device clock, and live memory/page-table addresses. The record was reviewed on September 10 UTC; the firmware clock is not used to establish a timezone or calibrated test time.

| Item | Observed result |
| --- | --- |
| Mode and result | DIAGNOSTIC; READY_OR_PATCHED |
| Overall status / candidate | EFI_SUCCESS; candidate_ready=1 |
| Target identity | One matching module; text CRC32 ED5F56DD; image size 0x5B40; target RVA 0x1030 |
| Memory type | EfiBootServicesCode (3) |
| Mapping | Valid, identity-mapped, supervisor, executable, writable; 2 MiB leaf page |
| CPU write protection | Enabled; no bypass needed |
| Memory Attribute Protocol | Absent; EFI_NOT_FOUND |
| Trigger at diagnostic entry | EFI_SUCCESS: marker already present |
| Patch / verification / rollback | All zero; diagnostic did not patch |
| Permission changes | clear_attempted=0; cleared_ro=0; restored_ro=0 |
| USB-log write | Screen photograph reported EFI_SUCCESS |

The absent-protocol branch in V4's inspection code requires a writable page-table mapping and a memory descriptor without EFI_MEMORY_RO. Successful inspection establishes both for this observed boot. `attributes_before=0` does not represent a successful attribute-protocol query when that protocol is absent.

The already-present trigger is acceptable for a diagnostic started as an ordinary boot application. It does not establish when a DriverOrder driver would run, and marker presence does not prove the warning itself was displayed. `trigger_before_patch`, clear status, and restore status remain EFI_NOT_READY because those operations were not attempted. PAGE_AFTER is zero because no post-patch page walk ran.

The earlier protection concern is not blocking the observed candidate path. The manual helper independently repeats identity and permission checks on its own boot. If that boot also starts with writable code and no attribute protocol, no permission clear/restore is required: a successful manual patch can correctly report both cleared_ro=0 and restored_ro=0.

Not supplied: confirmed power source, attached peripherals, prior test-driver-entry state, and a separately measured hash of the USB executable. The visible program version and log identify V4, and the helper's firmware identity checks passed; those are not a replacement for a USB file hash. Power conditions must be recorded with subsequent results.

Not established: physical write/readback success, automatic timing, warning suppression, permission-transition behavior, peripheral behavior, or removal/A-B behavior. [Issue #1](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/1) records the completed diagnostic review; [issue #2](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/2) tracks the next stage.
