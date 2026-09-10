# Project status — September 10, 2026

**First warning-free normal boot reported with the helper USB connected and no F12. Automatic RAM patching was verified in the preceding log. The new log, USB behavior, and with/without-helper comparison remain pending.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Both manual and automatic physical logs confirm expected identity, patched=1, verified=1, status=EFI_SUCCESS, and rollback=0. |
| Is the laptop's target page writable? | Yes in the diagnostic, manual, and first automatic logs: valid, executable supervisor mapping with a writable 2 MiB leaf page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | The first automatic log, supplied after USB DriverOrder registration, shows the marker absent at entry and before patching, followed by a verified patch. This supports the expected timing window for that run. A subsequent normal boot was reported warning-free; its fresh log and comparison with the helper absent remain pending. |
| Does the adapter warning disappear? | First warning-free normal boot reported in response to the USB-C test instructions, with the helper USB retained and no F12. Causal comparison and repeatability remain pending. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it, and it was not needed in the observed writable-page runs. Suitability and restoration remain unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

Physical evidence is recorded in the [diagnostic report](../hardware-results/phase-1-diagnostic-reviewed.md), [manual report](../hardware-results/phase-2-manual-reviewed.md), and [first automatic report](../hardware-results/phase-3-automatic-initial-reviewed.md). The installer screenshot showed an initially empty DriverOrder list followed by exactly one V4 TEST entry pointing to the USB helper. The subsequent automatic log has `status=0`, `patched=1`, `verified=1`, `rollback=0`, and `EFI_NOT_FOUND` for both trigger checks. The earlier diagnostic/manual logs instead found the trigger present.

A [subsequent observation](../hardware-results/phase-3-warning-free-reported.md) reports no adapter warning during normal startup, with the helper USB connected and no F12. It was supplied in response to the USB-C test instructions. The new run's driver log has not yet been received; the preceding automatic log remains separate evidence. Exact charger/cable details and normal Windows/USB peripheral behavior have not been explicitly recorded.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open for the fresh log, USB checks, and comparison with the helper absent. Preserve the new `LENOVO_V4_DRIVER.LOG` before further tests.

After saving work, fully shut down. Keep the same USB-C charger/cable connected and remove only the helper USB. Start normally without F12 and record whether the warning returns; press Esc if it appears. Fully shut down again, reinsert the helper USB into the same port, and boot normally. Record whether the warning disappears again and collect the new driver log. Do not change power or other boot settings between the comparison runs.

Also check ordinary USB keyboard/mouse/storage/dock behavior. If the warning does not follow the expected absent/present-helper comparison, record the actual result. Firmware may clean up a missing-media DriverOrder entry; if reinsertion does not resume the helper, inspect the current list before reinstalling and avoid duplicate entries. If startup hangs, fully power off, remove the helper USB, and boot without it.

The helper stays on USB and its RAM patch lasts only that boot. The saved DriverOrder entry may remain until removed. Secure Boot trust, internal installation, and a read-only permission transition have not been tested or implemented beyond the original package. V4 source, helper binaries, and published release assets remain unchanged.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
