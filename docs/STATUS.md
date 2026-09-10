# Project status — September 10, 2026

**Phase 1 complete: full physical diagnostic log reviewed; correct target found and page writable. The existing Phase 2 manual RAM test is the next step. Physical patching and warning suppression remain untested.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Full physical diagnostic log confirms one matching module, expected text CRC32 ED5F56DD and a ready candidate. Physical patch result pending. |
| Is the laptop's target page writable? | Yes in the reviewed diagnostic boot: valid, executable supervisor mapping with a writable 2 MiB leaf page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | Static BdsDxe ordering is promising. DriverOrder execution on the laptop remains untested. An F12 diagnostic cannot settle this. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it; suitability and restoration are unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

The [Phase 1 report](../hardware-results/phase-1-diagnostic-reviewed.md) records the reviewed log and its limits; [issue #1](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/1) is complete. The log confirms a ready target, writable executable supervisor mapping, absent Memory Attribute Protocol, and no patch or permission change. The trigger was already present at diagnostic entry, which is acceptable for this stage and does not measure DriverOrder timing. Unattempted-operation status fields and PAGE_AFTER remain at their initialized values.

Proceed to the existing Phase 2 manual test after reviewing its walkthrough, then review `LENOVO_V4_MANUAL.LOG`. Expected success is `READY_OR_PATCHED`, `status=0`, `patched=1`, `verified=1`, and `rollback=0`. If permissions remain as observed, `cleared_ro=0` and `restored_ro=0` are expected because no read-only transition is needed. If a transition is attempted, successful restoration remains mandatory. Record the power source for both diagnostic and manual runs; it has not been confirmed for the diagnostic.

For the manual RAM mechanics test, a sufficiently charged battery with both charging adapters disconnected is acceptable: the helper has no AC-presence requirement. The normal Lenovo charger was the conservative power baseline, not a firmware-programming requirement. The unchanged V4 screen still recommends that charger. Identity and permissions are checked again on every run; a different power state may change firmware behavior. Battery-only testing does not substitute for the later USB-C warning test or establish behavior with the normal charger.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
