# Project status — September 10, 2026

**Helper-dependent warning suppression demonstrated on the tested laptop: warning absent with the helper USB, present after removal, and absent after reinsertion. The latest automatic log confirms another early verified patch. USB peripheral checks remain pending. Internal-drive deployment revision 1 is now prepared for its first physical installation.**

| Question | Evidence and current conclusion |
| --- | --- |
| Is there an easy warning-off NVRAM preference? | None established in the analyzed image. `IllegalAdapter` is status, not an identified disable preference. |
| Can V4 recognize and patch its intended target? | Both manual and automatic physical logs confirm expected identity, patched=1, verified=1, status=EFI_SUCCESS, and rollback=0. |
| Is the laptop's target page writable? | Yes in the diagnostic, manual, and both reviewed automatic logs: valid, executable supervisor mapping with a writable 2 MiB leaf page. The Memory Attribute Protocol lookup returned EFI_NOT_FOUND. Other boot paths must be checked independently. |
| Can the automatic driver run early enough? | The first automatic log, supplied after USB DriverOrder registration, shows the marker absent at entry and before patching, followed by a verified patch. This supports the expected timing window for that run. Subsequent normal boots were reported warning-free with the helper, with the warning returning when the helper USB was removed. |
| Does the adapter warning disappear? | Yes in the reported with/without/with comparison: absent with the helper USB, present without it, absent again after reinsertion. The report follows the same-USB-C-power comparison instructions; broader repeatability remains untested. |
| Does skipping the callback affect USB behavior? | Unknown. The skipped routine also contains a USB controller connection call; later physical testing must check peripherals. |
| Is the older CPU interface useful? | A SetMemoryAttributes implementation was identified statically. V4 does not use it, and it was not needed in the observed writable-page runs. Suitability and restoration remain unproven. |
| Can the helper keep running after Secure Boot is re-enabled? | No trust arrangement has been established for these unsigned helpers. |

## Validation already recorded

The original V4 package records 33 successful sanitized host cases, byte-identical rebuilds, binary inspection, and an isolated QEMU/OVMF integration run. The emulator's native read-only permission-transition test was **skipped** because that protocol was unavailable; mock coverage does not replace a physical test. Details: [VALIDATION.md](VALIDATION.md).

Public repository checks run package integrity and deterministic builds. The 33 host cases run against the reviewed inert module fixture. GitHub workflow outcomes are recorded in Actions. See [repository setup validation](REPOSITORY-VALIDATION.md) for the local result.

## Immediate next evidence

Physical evidence is recorded in the [diagnostic report](../hardware-results/phase-1-diagnostic-reviewed.md), [manual report](../hardware-results/phase-2-manual-reviewed.md), and [first automatic report](../hardware-results/phase-3-automatic-initial-reviewed.md). The installer screenshot showed an initially empty DriverOrder list followed by exactly one V4 TEST entry pointing to the USB helper. The subsequent automatic log has `status=0`, `patched=1`, `verified=1`, `rollback=0`, and `EFI_NOT_FOUND` for both trigger checks. The earlier diagnostic/manual logs instead found the trigger present.

A [subsequent observation](../hardware-results/phase-3-warning-free-reported.md) reported no warning on normal startup with the helper USB and no F12. The [shutdown/start comparison](../hardware-results/phase-3-usb-comparison-reported.md) then reproduced the expected sequence: warning returned without the helper USB and disappeared when it was reinserted. These user observations establish the desired behavior in the tested setup. A later full automatic log was then supplied and reviewed: both trigger lookups returned EFI_NOT_FOUND, patched=1, verified=1, status=EFI_SUCCESS, rollback=0, and no permission changes were attempted. This agrees with the earlier successful automatic log.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open for ordinary USB keyboard/mouse/storage/dock checks. The latest driver log has been received and reviewed. No further repeated startup cycles are required merely to restate the completed comparison. Record the actual peripheral observations and retain the newest `LENOVO_V4_DRIVER.LOG`.

The exact charger/cable model, a separately measured USB executable hash, and long-term behavior have not been documented. The comparison instructions held the USB-C power setup constant and changed only the helper USB; no deviation was reported. Do not generalize this result to other BIOS versions, machines, or power/boot conditions without evidence.

The owner requested normal startup without the helper USB. [Internal deployment revision 1](../deployment/internal-1/READ-ME-FIRST.md) supplies a dedicated internal EFI folder, copy/readback script, explicit saved-entry migration, and disable/recovery instructions. It reuses the unchanged working helper. The new scripts are checked in isolated QEMU/OVMF; the internal startup and Windows recovery path still need physical confirmation. This preparation does not depend on repeating the completed USB comparison. Check normal peripherals during the next startup.

The currently demonstrated setup still loads from USB. The internal package is ready to migrate the file and saved DriverOrder location; its RAM patch likewise lasts only the current boot and is reapplied automatically on each startup. No Secure Boot trust arrangement or physical read-only permission transition has been established. [Secure Boot research](SECURE-BOOT-OPTIONS.md) records exact-image-approval strings present in the supplied firmware, with menu availability and enforcement behavior unproven. V4 source, helper binaries, and the original public release assets remain unchanged.

Future steps are in the [tracking issues](../planning/README.md). They are dependencies to investigate, not claims that every stage will work.
