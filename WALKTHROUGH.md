# V4 USB test walkthrough

Read `START-HERE.md` first. Only GKCN65WW on the stated Lenovo model has been analyzed. No successful physical-laptop result is claimed.

## Phase 1: read-only diagnostic

Use the normal Lenovo charger. Check the BIOS version and current encryption status as described in `START-HERE.md`. Disable Secure Boot without clearing its keys.

Use a spare FAT32 USB. If formatting is necessary, back up that USB first: formatting erases it. This package does not format disks or run a disk-selection script. NTFS/exFAT are not substitutes for this test. If Windows does not offer FAT32, use another suitable FAT32 stick or ask for help identifying that USB before formatting.

Copy the **contents** of `01_DIAGNOSTIC_USB` to the USB root. Keep other useful files and old logs backed up. `EFI\BOOT\BOOTX64.EFI` and `LenovoWarning-v4-Diagnostic.efi` are identical; the former is the standard removable-media boot filename.

F12-boot the UEFI USB entry. It displays a compact result and appends a detailed record to `LENOVO_V4_DIAGNOSTIC.LOG`. Send that log for review before Phase 2. If writing fails, take a photo of the whole result.

Expected candidate result:

- `result=READY_OR_PATCHED`, `candidate_ready=1`, `module_handles=...0001`.
- `text_crc32=0x00000000ED5F56DD`, a valid identity-mapped executable supervisor page.
- `patched=0` and `verified=0`: correct for this read-only phase.
- Either the page is already writable, or a successful attribute query reports RO and both clear/restore methods exist. The latter is only a candidate; Phase 2 must prove the transition on the laptop.

Any other result stops progression. `TARGET_NOT_FOUND` can mean the module is unavailable or its firmware GUID is not exposed. `IDENTITY_MISMATCH` can mean a different module build, an earlier RAM patch, or a changed code section. Do not force a match or alter the fingerprint.

The trigger may already be present in a diagnostic launched from F12. That does not invalidate Phase 1: an application runs later than the planned automatic driver.

## Phase 2: manual RAM patch mechanics

After the Phase 1 result has been reviewed, keep the Lenovo charger connected. Copy the contents of `02_MANUAL_PATCH_USB` onto the same USB, replacing `EFI\BOOT\BOOTX64.EFI`. No reformat is necessary. Retain the Phase 1 log.

F12-boot that USB. The manual application asks for **uppercase P** before running; any other key exits. It repeats identity and permission checks independently of the earlier diagnostic.

Success requires:

- `result=READY_OR_PATCHED`, `status=0x0000000000000000`.
- `patched=1`, `verified=1`, `rollback=0`.
- If `cleared_ro=1`, also `restored_ro=1`.

It writes `LENOVO_V4_MANUAL.LOG`. Photograph the screen if the log cannot be written. After the test, fully power off. This manual test establishes write mechanics; it does not establish early-enough execution or suppression of the real warning.

On any error or freeze, stop. Fully power off, remove the USB, then boot normally. If restoration failed, a byte change or permission change may remain for that boot; a fresh boot discards this test's RAM state. The stored BIOS was not flashed. Send the log before trying another patch.

## Phase 3: automatic driver from the USB

Only after Phase 2 succeeds, copy the contents of `03_AUTOMATIC_DRIVER_TEST` onto the USB. This replaces `EFI\BOOT\BOOTX64.EFI` with the bundled official TianoCore x64 UEFI Shell. The automatic helper is `LenovoWarning-v4-Driver.efi` at the USB root.

The diagnostic/manual application and the automatic driver have different PE subsystem types. Renaming the manual application does not turn it into a driver.

With the Lenovo charger connected and Secure Boot disabled, F12-boot the USB. At the Shell:

```text
map -r
fs0:
ls
```

If that filesystem does not contain `LenovoWarning-v4-Driver.efi` and `install-driver.nsh`, try `fs1:`, `fs2:`, etc. Identify the USB by its files; do not assume it is always fs0.

Run:

```text
install-driver.nsh
```

Without an argument, the script only displays DriverOrder and instructions. Review the list. If an earlier entry named `Lenovo USB-C Warning Patch v2 TEST`, `... v3 TEST`, or `... v4 TEST` exists, remove that exact entry by its **current Option/list position** using `bcfg driver rm <position>`. Re-display the list after each removal. Preserve unrelated entries. Do not leave two patchers competing.

Then run once:

```text
install-driver.nsh INSTALL
```

This adds `Lenovo USB-C Warning Patch v4 TEST` at the beginning of DriverOrder, pointing to the USB file. Verify the final dump shows **one** such entry and no command reported an error. The script does not reboot automatically. Do not repeat INSTALL if the entry already exists.

Type `reset` when ready. Leave the USB plugged into the same port. Let the laptop boot normally; **do not press F12**. The helper is silent; a Shell or helper screen during ordinary startup is not the intended result.

### First automatic boot: Lenovo charger

Confirm normal Windows startup and inspect `LENOVO_V4_DRIVER.LOG` on the USB. A valid new record must contain:

```text
mode=AUTOMATIC_DRIVER
result=READY_OR_PATCHED
status=0x0000000000000000
trigger_before_patch=0x800000000000000E
patched=1
verified=1
rollback=0
```

`...000E` is EFI_NOT_FOUND: the warning-trigger protocol was not installed when the helper checked. `cleared_ro=1` additionally requires `restored_ro=1`.

Records append. Use the **last complete BEGIN/END record** and its firmware clock fields. The clock may be inaccurate. For an unambiguous new test, archive/remove the old driver log in Windows before the next boot; this does not change the patch. Logs are capped at about 1 MiB; archive a full log before continuing. No log is not proof of failure or success: logging is best-effort. Stop and diagnose if a fresh complete record is missing.

If a Shell appears instead of Windows, choose Windows Boot Manager for that boot and check BIOS boot priority; removable boot media can affect the firmware's boot selection. The helper does not modify BootOrder or the Windows loader.

### Real USB-C warning test

After the Lenovo-charger automatic test passes:

1. Archive/remove the old driver log in Windows.
2. Fully shut down, for example with `shutdown /s /t 0` after saving work. Avoid relying on a Windows Fast Startup/hybrid shutdown for this test.
3. Keep the USB inserted. Disconnect the Lenovo charger and connect the same USB-C PD charger that reliably produces the warning.
4. Start normally without F12. Check whether Windows starts without the warning, and inspect the fresh driver log.

The `/hybrid` option is separate from this shutdown command. [Microsoft shutdown command reference](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/shutdown).

If the warning appears, press Esc to continue and send the log. `WARNING_TRIGGER_ALREADY_PRESENT` means the helper found the marker installed and deliberately refused a late patch. A successful patch with no marker present, yet a visible warning, needs further investigation; it does not prove a specific cause by itself.

Also check normal USB keyboard/dock operation. The patched callback performs a USB-controller connection step before displaying the warning, so bypassing the entire callback skips that step too. The physical impact of that omission is not established by static analysis or emulation.

### A/B verification

If the warning disappears:

1. Fully shut down, remove the USB, keep the same USB-C charger, and boot again. The original warning should return.
2. Fully shut down, reinsert the USB in the same port, and boot again. The warning should disappear with a fresh successful driver log.

This establishes that the change depends on the USB helper. It does not increase USB-C power, bypass charger identification, or remove the laptop's power limits.

## Removal and recovery

For a hang, fully power off and unplug the USB before starting again. Do not repeatedly retry a failing patch. The executable is on the removable USB; a stale DriverOrder entry may remain and can be removed afterward. Firmware may maintain or remove missing-media entries itself.

For normal removal, F12-boot the Phase 3 Shell and run:

```text
bcfg driver dump -v
```

Find the exact V4 TEST description. Remove its **current list position**, not the hexadecimal Driver#### identifier:

```text
bcfg driver rm <current-position>
bcfg driver dump -v
```

Confirm it is gone. Never blindly use position 0. If it is already absent, do not remove something else. The same instructions are in `REMOVE-DRIVER.txt` on the Phase 3 USB.

Re-enable Secure Boot after removing the unsigned helper if ending testing. The unsigned helper will not keep running with Secure Boot re-enabled. A supported signing/trust setup would be a separate task; factory key replacement is not part of this package.

There is no BIOS flasher, EC tool, hidden-variable writer, Windows bootloader replacement, or internal-ESP installer here. After successful physical A/B testing, an internal installation and Secure Boot approach can be designed against the verified result.
