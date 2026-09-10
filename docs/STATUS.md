# Project status — September 10, 2026

**Automatic RAM patch verified: the trigger was absent at both automatic-driver checks and the byte change passed readback. Actual USB-C warning suppression, USB behavior, and repeatability remain untested.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Both manual and automatic physical logs confirm expected identity, patched=1, verified=1, status=EFI_SUCCESS, and rollback=0. |
| Is the laptop's target page writable? | Yes in the diagnostic, manual, and first automatic logs: valid, executable supervisor mapping with a writable 2 MiB leaf page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | The first automatic log, supplied after USB DriverOrder registration, shows the marker absent at entry and before patching, followed by a verified patch. This supports the expected timing window for that run; warning behavior and other boot conditions remain unverified. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it, and it was not needed in the observed writable-page runs. Suitability and restoration remain unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

Physical evidence is recorded in the [diagnostic report](../hardware-results/phase-1-diagnostic-reviewed.md), [manual report](../hardware-results/phase-2-manual-reviewed.md), and [first automatic report](../hardware-results/phase-3-automatic-initial-reviewed.md). The installer screenshot showed an initially empty DriverOrder list followed by exactly one V4 TEST entry pointing to the USB helper. The subsequent automatic log has `status=0`, `patched=1`, `verified=1`, `rollback=0`, and `EFI_NOT_FOUND` for both trigger checks. The earlier diagnostic/manual logs instead found the trigger present.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open for actual warning suppression, USB behavior, and the comparison with the helper absent. The successful automatic log does not establish those observations. Power source and a direct report of normal Windows startup have not yet been supplied.

For the next run, preserve the existing driver log under another name so the helper creates a fresh `LENOVO_V4_DRIVER.LOG`. Save work and fully shut down. Keep the helper USB in the same port, disconnect the normal Lenovo charger if attached, and connect the USB-C charger and cable that reliably produce the warning. Start normally without F12. Record whether the adapter warning appears, whether Windows starts normally, and whether ordinary USB keyboard/mouse/storage/dock functions work. Send the fresh driver log. If the preceding automatic run already used USB-C, record that fact and the observed warning behavior rather than infer the power source from the log.

If the warning appears, press Esc to continue and inspect the log. If startup hangs, fully power off, remove the helper USB and boot without it. Do not reinstall or add duplicate entries merely because a log is missing. After a successful USB-C run, the planned powered-off USB-removal/reinsertion comparison can test whether suppression depends on the helper. Firmware may remove missing-media entries; if reinsertion does not restore operation, inspect the current DriverOrder state before drawing a conclusion.

The helper stays on USB and its RAM patch lasts only that boot. The saved DriverOrder entry may remain until removed. Secure Boot trust, internal installation, and a read-only permission transition have not been tested or implemented beyond the original package. V4 source, helper binaries, and published release assets remain unchanged.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
