# Project status — September 10, 2026

**Phases 1 and 2 passed: physical target recognition and manual one-byte RAM patch/readback verified. Automatic timing, warning suppression, and USB behavior remain untested.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Full manual log confirms the expected identity, patched=1, verified=1, status=EFI_SUCCESS, and rollback=0 on the physical laptop. |
| Is the laptop's target page writable? | Yes in both reviewed boots: valid, executable supervisor mapping with a writable 2 MiB leaf page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | Static BdsDxe ordering is promising. DriverOrder execution on the laptop remains untested. An F12 diagnostic cannot settle this. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it; suitability and restoration are unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

The [Phase 1 report](../hardware-results/phase-1-diagnostic-reviewed.md) records successful diagnosis; the [Phase 2 report](../hardware-results/phase-2-manual-reviewed.md) records successful physical byte change/readback with no rollback and no permission transition. Issues [#1](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/1) and [#2](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/2) are complete for those test outcomes.

The trigger was already present at entry and before the manual patch. Manual mode permits this; automatic mode will refuse it. The next evidence is a fresh `LENOVO_V4_DRIVER.LOG` from a normal startup after registering the USB helper in DriverOrder. Target success includes `mode=AUTOMATIC_DRIVER`, `status=0`, `patched=1`, `verified=1`, `rollback=0`, and `trigger_before_patch=0x800000000000000E` (marker absent). Observed warning suppression and USB behavior still need separate testing.

Before registering the driver, fully power off after the manual test, remove the USB, and verify normal Windows startup. That recovery/normal-boot observation has not yet been explicitly reported. The normal Lenovo charger is the recommended baseline, but a sufficiently charged battery alone is acceptable for the initial non-USB-C loading test. Record which was used. This does not substitute for the later USB-C warning test or establish behavior under a different power condition.

Follow the Phase 3 section of [WALKTHROUGH.md](../WALKTHROUGH.md): copy the Phase 3 folder contents, enter the bundled Shell, identify the USB, inspect DriverOrder with the no-argument installer, and add the test entry only if no competing Lenovo test entry exists. Preserve unrelated entries. If the existing list is unclear, review it before any change. The exact V4 entry must appear once. Leave the USB in the same port and boot normally for the automatic test. A missing log is inconclusive; do not repeatedly add entries to fix it.

If startup hangs, fully power off, remove the USB and boot without it. The saved DriverOrder entry may remain; use the exact-description/current-position removal instructions when ending the test. The helper stays on USB and its RAM patch lasts only that boot. No full BIOS flash, internal installation, or Secure Boot trust change has been added.

Power sources and previous-driver-entry state for the completed tests remain unconfirmed. The supplied logs show successful V4 execution; they do not encode those missing observations. V4 helper source, binaries, and published package remain unchanged.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
