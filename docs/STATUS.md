# Project status — September 10, 2026

**First physical diagnostic screen reviewed: correct target found and page writable. A physical patch and warning suppression remain untested. Full diagnostic log pending.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Physical diagnostic screenshot shows one matching module and expected text CRC32 ED5F56DD. Physical patch result pending. |
| Is the laptop's target page writable? | Yes in the photographed diagnostic boot: valid mapping and writable page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | Static BdsDxe ordering is promising. DriverOrder execution on the laptop remains untested. An F12 diagnostic cannot settle this. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it; suitability and restoration are unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

The [Phase 1 issue](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/1) records the screen transcription. The screenshot shows `READY_OR_PATCHED`, one matching module, CRC32 `ED5F56DD`, valid/writable page, absent attribute protocol, no patch, and a successful USB-log write. The image and live addresses are not published. The power source for this observation has not been confirmed.

Obtain `LENOVO_V4_DIAGNOSTIC.LOG` for the complete record. The screen supports the manual-test candidate path, but it does not establish automatic timing or warning suppression.

For the manual RAM mechanics test, a sufficiently charged battery with both charging adapters disconnected is acceptable: the helper has no AC-presence requirement. The normal Lenovo charger was the conservative power baseline, not a firmware-programming requirement. The unchanged V4 screen still recommends that charger. Identity and permissions are checked again on every run; a different power state may change firmware behavior. Battery-only testing does not substitute for the later USB-C warning test or establish behavior with the normal charger.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
