# Project status — September 10, 2026

**Physical outcome: unknown. No reviewed physical-laptop result has been recorded.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Existing host and synthetic OVMF tests passed within their documented scope. Physical result pending. |
| Is the laptop's target page writable? | Unknown until the live diagnostic. The newer Memory Attribute Protocol GUID was absent from the decompressed-module scan. |
| Can the automatic driver run early enough? | Static BdsDxe ordering is promising. DriverOrder execution on the laptop remains untested. An F12 diagnostic cannot settle this. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it; suitability and restoration are unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

Phase 1 uses `01_DIAGNOSTIC_USB` and the normal Lenovo charger. Read-only means the helper inspects firmware state; it still writes its text log to USB. Review `LENOVO_V4_DIAGNOSTIC.LOG` before deciding on the manual experiment. Do not mark timing, warning suppression, or successful removal as passed based on this log alone.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
