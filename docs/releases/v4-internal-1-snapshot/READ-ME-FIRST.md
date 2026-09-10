# Move the working V4 helper onto the internal drive

**After this installation succeeds, normal startup will not need the USB stick.** You use the USB once for setup, then keep it somewhere as a recovery tool.

This guide is for the Lenovo Legion 5 Pro 16ACH6H / 82JQ with **GKCN65WW**, after the V4 automatic USB experiment has already worked. It uses the exact same helper. Internal-drive startup is a new deployment location and still needs its first physical confirmation; the completed USB comparison already established warning suppression.

## What you are doing

Your internal drive has a small EFI system partition containing Windows' startup files. We add one separate folder:

```text
\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
```

We then replace the project's USB driver entry with an internal driver entry. The computer will load the helper automatically before Windows and apply the same RAM patch each boot. There is no BIOS flash, Windows reinstall, repartitioning, or replacement of Windows Boot Manager. NVRAM holds a saved file location, not the warning patch itself.

The helper is still unsigned. **Keep Secure Boot disabled during this installation.** Signing or approving this exact file is separate work; do not clear or replace Secure Boot keys during these steps.

## Before starting

- Keep this guide open on another device, or print it.
- Have your BitLocker/device-encryption recovery key available privately if encryption is enabled. Do not post it. Changing boot configuration can cause a recovery prompt.
- Keep the existing working Phase 3 USB. Do not format it. Copy the contents of this package's `USB` folder into the root of that USB. The included helper and Shell are the same bytes as before; the new files are the two `.nsh` scripts.
- In Windows PowerShell, verify the helper on the USB, substituting your actual USB letter for `E:`:

```powershell
Get-FileHash -Algorithm SHA256 'E:\LenovoWarning-v4-Driver.efi'
```

The hash must be:

```text
665D32265CE91946E58F9910F366DB29682970D68BA4BCBA7B749104C4382898
```

If it differs, do not use that file. Extract the package again and check the published archive checksum.

Use a charged battery and a stable power setup. The barrel charger is suitable for installation; charged battery alone also avoids the low-power-adapter prompt. For the final warning check, use the same USB-C charger/cable that produced the original warning. This patch does not increase available power.

## 1. Open the same UEFI Shell

Shut down, leave the setup USB inserted, and use F12 to start its UEFI Shell as you did before. If `startup.nsh` is offered, press Esc to skip it. This package does not install an automatic startup script.

Run:

```text
map -r
map -v
```

`FS0:`, `FS1:`, etc. are temporary volume names. They can change between boots. **Do not assume that FS0 is internal or FS1 is USB.**

Identify both volumes:

- The USB contains `stage-internal.nsh` and the V4 helper in its root. Its device path includes `USB`.
- The internal Windows EFI volume contains `\EFI\Microsoft\Boot\bootmgfw.efi`. In `map -v`, its hardware path must belong to the internal SSD, not USB. The Windows EFI volume is a small FAT partition; the large Windows data partition may not be readable by this Shell.

For example, inspect candidate volumes with:

```text
ls fs0:\EFI\Microsoft\Boot\bootmgfw.efi
ls fs1:\stage-internal.nsh
```

**The remaining examples assume you have confirmed internal = FS0 and USB = FS1. Substitute the actual numbers throughout.** If multiple volumes contain Windows Boot Manager and you cannot identify the active internal Windows EFI partition, stop before copying and send the volume list for review. Scrub partition identifiers from public posts.

Select the USB and its root:

```text
fs1:
cd \
```

## 2. Copy the helper internally

First perform a check without copying:

```text
stage-internal.nsh fs0:
```

It should display the intended internal destination, the existing driver list, and `CHECK_ONLY`. Then run:

```text
stage-internal.nsh fs0: COPY
```

The script creates only the project folder, copies the helper, and compares both files byte for byte. **Continue only if it ends with `STAGED_OK` and no command reports an error.** It does not yet change a saved driver entry.

`STOP_EXISTS` means the dedicated folder already exists. Nothing was overwritten. If this is a second attempt after `STAGED_OK`, verify the existing copy before continuing:

```text
comp LenovoWarning-v4-Driver.efi fs0:\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
```

Continue only if `comp` reports no differences and the file came from this verified package. If an earlier copy failed, leave the existing folder for inspection; do not register a partial or unverified file.

## 3. Replace the USB startup entry

Run:

```text
bcfg driver dump -v
```

Find the entry described as **Lenovo USB-C Warning Patch v4 TEST** and check that its path points to the USB helper. Remove only that entry, using its **current displayed Option position**. In your earlier screenshot it was Option 00; if it is still Option 00, the command is:

```text
bcfg driver rm 0
```

If its Option position is different, use that position instead. The `Driver0000` variable name is not the removal position. Do not remove Windows boot entries or unrelated drivers. If an INTERNAL entry already exists, do not add a duplicate; inspect its path and finish with exactly one project entry.

Once the USB TEST entry is gone, add the internal copy:

```text
bcfg driver add 0 fs0:\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi "Lenovo USB-C Warning Patch v4 INTERNAL"
bcfg driver dump -v
```

Verify that there is **exactly one project entry**, named INTERNAL, and its path points to the internal SSD and the project folder. The old USB TEST entry must be absent. No command should report an error. Keep unrelated entries intact.

If registration fails, the BIOS has not been flashed. The warning can return until an entry is fixed. You can restore the prior USB arrangement using the original `install-driver.nsh INSTALL` from its USB root, after checking the driver list for duplicates.

## 4. Shut down and put the USB away

In the Shell:

```text
reset -s
```

Wait for the computer to turn off fully. Remove the helper USB. Connect the same USB-C charger/cable used in the successful comparison, and start normally without F12.

Expected result: Windows starts with no adapter warning and no helper USB present. Report what happened. Also check the USB devices you normally use, such as a mouse, storage device, or dock. The skipped firmware routine includes a USB connection call, so ordinary peripheral behavior is the remaining functional check.

There is no reason to repeat the completed USB with/without/with experiment. This startup checks only the new internal location.

## Reading the internal log

The unchanged helper appends to `\LENOVO_V4_DRIVER.LOG` on the volume it loaded from. After this move, that is the internal EFI partition, not the USB. It stops appending once the previous file length exceeds 1 MiB, so the file is bounded to roughly 1 MiB plus one report. A log-write failure does not stop patching or Windows startup.

If a log is needed, use **Command Prompt as administrator** in Windows. First make sure `S:` is unused, including network and virtual drives. If it is in use, choose another unused letter and substitute it in every command. Do not use a letter already assigned to a volume.

```bat
mountvol S: /S
dir S:\EFI\Microsoft\Boot\bootmgfw.efi
dir S:\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
copy S:\LENOVO_V4_DRIVER.LOG "%USERPROFILE%\Lenovo-V4-internal-results.txt"
mountvol S: /D
```

Run these one at a time. If mounting fails, stop; do not run commands against a different volume. If the expected paths are absent, remove the temporary mount with `mountvol S: /D` and stop. The last command only removes the temporary drive letter; it does not delete the EFI partition. The copied text is in your Windows user folder. Send it privately for review; do not post the raw log publicly.

## Disable or remove the internal setup

**Removing the USB no longer disables an internal installation.** You disable the internal file or remove its saved entry instead. Any RAM change from the current boot lasts until shutdown.

With the recovery USB, open the Shell and identify the volumes again. From the USB root, substituting the actual internal volume:

```text
disable-internal.nsh fs0: DISABLE
```

Continue only if it says `DISABLED_OK`. This renames only the project helper to `LenovoWarning-v4-Driver.disabled`, making the saved internal path unavailable on the next boot. It does not erase files or change NVRAM. From the displayed driver list, remove the **INTERNAL entry's current Option position** using `bcfg driver rm` as explained above. Leave unrelated entries alone. Shut down with `reset -s`, remove the USB, and restart. The stock warning behavior should return.

The small disabled file can stay there harmlessly. If you want all project files removed, first confirm the INTERNAL entry is gone, then delete only that disabled file and the now-empty `\EFI\LenovoAdapterWarningV4` folder. The log is separate; delete only `\LENOVO_V4_DRIVER.LOG` after keeping any results you need. Never delete the EFI or Microsoft folders.

### If you need to disable it from Windows

Open Command Prompt as administrator. With `S:` confirmed unused, run these commands one at a time:

```bat
mountvol S: /S
dir S:\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
ren S:\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi LenovoWarning-v4-Driver.disabled
mountvol S: /D
shutdown /s /t 0
```

If mounting fails or the expected helper is absent, stop and unmount the temporary letter if it was created. If a `.disabled` file already exists, renaming refuses to overwrite it; inspect the folder before doing anything else. The saved driver entry will remain but cannot load the renamed file. Remove that exact entry later through the Shell.

If startup hangs before you can reach the Shell, an internal helper can run before the F12 menu too. Try entering Lenovo firmware settings with F2/Fn+F2 at power-on and enabling Secure Boot **without clearing keys**. With the current unsigned, unenrolled helper this should block it, allowing the usual trusted Windows boot path. It will also block the unsigned recovery Shell. If Windows then starts, disable the internal file using the steps above. The exact Lenovo recovery path has not been physically tested; if firmware settings or Windows remain inaccessible, stop and report what the machine displays. Do not start a BIOS flash as a recovery guess.

That Secure Boot recovery method must be reconsidered if we later enroll or sign the helper: a trusted helper would no longer be blocked merely by enabling Secure Boot.

## Risk in plain language

This avoids the main flash-bricking hazard because it does not rewrite the BIOS chip. The helper has already worked on this laptop from USB. Nevertheless, software that runs before Windows can cause a hang, an incorrect driver entry can fail to load, a power interruption during an EFI filesystem write can corrupt files, and careless manual deletion can damage Windows startup. Risk is not zero, and no meaningful percentage has been measured.

The new script limits its copy to a fresh dedicated folder and refuses to overwrite an existing one. It verifies the copied bytes. Saved-entry changes remain explicit, visible Shell commands. Internal deployment loses the USB-removal recovery shortcut, which is why the disabling instructions above matter.

A BIOS update may change the target and cause V4 to refuse patching. Firmware resets can remove the saved entry. Re-enabling Secure Boot without establishing trust will stop this unsigned helper. Those events can bring the warning back; the helper does not bypass its identity or permission checks.

## What the checks mean

The new Shell scripts are exercised in an isolated QEMU/OVMF virtual computer with disposable disk-image files. It checks copy/compare behavior, refusal paths, disabling, and the ordinary `bcfg` commands. It does not emulate this Lenovo's charging system, test Windows recovery, or prove internal-drive timing on the laptop. The V4 core's existing 33 host cases and deterministic binary checks remain separate.

Primary references: [UEFI Shell specification](https://uefi.org/sites/default/files/resources/UEFI_Shell_2_2.pdf), [Microsoft mountvol documentation](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/mountvol), and [Secure Boot databases](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).
