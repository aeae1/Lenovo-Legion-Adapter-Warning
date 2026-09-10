# Lenovo V4: the beginner walkthrough

For your Lenovo Legion 5 Pro 16ACH6H / 82JQ with BIOS **GKCN65WW**. This guide accompanies **Lenovo-GKCN65WW-V4-Public-USB-Test.zip**. You do not need to know how to program, compile anything, or understand the hexadecimal numbers in the logs.

**This is an experimental workaround. It has passed software tests and UEFI emulation, but it has not yet been demonstrated on your physical Lenovo. Start with the diagnostic and share its result before attempting the patch.**

## 1. What we are trying to accomplish

When you start the laptop, its motherboard software runs before Windows. People usually call this software the BIOS; the modern system is called UEFI. It checks and initializes hardware, then starts Windows.

Your adapter message is displayed during that early startup. A normal Windows program starts too late to prevent it.

We found the routine responsible for displaying the warning. The proposed helper runs before Windows and changes the beginning of that routine **in the temporary copy loaded into RAM**. The change means, roughly, “return immediately instead of running this routine.”

The intended visible result is the Lenovo logo followed by Windows, without the adapter interruption. It does not turn a 100 W USB-C charger into a higher-power charger or remove charging/performance limits.

There are three different kinds of storage involved:

| Area | What it means | What this procedure does |
|---|---|---|
| Stored BIOS program | The motherboard's saved startup software | The package does not replace this program or flash a modified BIOS image. |
| RAM | Temporary working memory used during a running session | The patch changes one byte in the loaded warning routine. A fresh boot without the helper starts with the original code again. |
| Saved firmware settings, also called NVRAM | Settings that survive shutdown | You change Secure Boot in BIOS Setup. In the automatic phase, the Shell adds a standard startup entry telling firmware where to find the USB helper. These settings persist until changed or removed. |

The automatic entry is called `DriverOrder`/`Driver####`. Think of it as an early-startup shortcut pointing to a file on the USB. It is not a newly discovered “disable adapter warning” setting. [UEFI describes these standard driver load options here.](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html)

Because the code change is temporary, the helper must run on each fresh boot for suppression to continue. The current package keeps the helper on a removable USB. If the tests succeed, putting it on internal storage would be a later, separate step. Keeping Secure Boot enabled would also need a supported trust arrangement that has not been established yet.

## 2. What is the brick potential?

**My assessment is that the expected permanent-brick risk is low relative to replacing the stored BIOS, because this procedure avoids that write. That is an engineering judgment, not a measured failure rate or a guarantee.** I cannot honestly give you a percentage.

A “hard brick” means the laptop cannot even reach normal firmware startup and might require a manufacturer recovery method, firmware-chip programming or repair. A boot freeze, an unexpected boot screen, or a BitLocker recovery prompt can look alarming without being a hard brick.

| Step | What changes | Main realistic concerns |
|---|---|---|
| Read-only diagnostic | It inspects firmware and writes a report on the USB. It makes ordinary temporary allocations but does not deliberately patch existing firmware code or edit firmware settings. | It could fail to run, fail to find the target, fail to log, or freeze due to a software/firmware interaction. It runs before Windows with substantial privileges, so “read-only” is not a zero-risk guarantee. |
| Manual RAM patch | Changes the loaded callback; if needed and supported, temporarily changes write protection on one memory page and attempts to restore it. | Incorrect behavior, a freeze, or failed permission restoration during that boot. This is more consequential than the diagnostic. |
| Automatic USB test | Adds a persistent firmware startup entry, then performs the same RAM patch during ordinary boot. | Boot freezes can repeat while the USB helper remains available; USB discovery or boot selection may behave differently; firmware-variable operations are not entirely risk-free. |
| Preparing the USB | You may format a spare USB and copy files to it. | Formatting the wrong drive or failing to back up its contents would lose data. The package does not format drives automatically. |

The amount changed—one byte—is not itself a safety guarantee. One byte can change program behavior significantly. The safeguards are the exact target checks, the limited scope, the permission checks, staged testing, and the removable helper.

There is also one concrete possible side effect: the warning routine performs a USB-controller connection step before showing the message. Bypassing the whole routine skips that step too. We must check normal USB keyboard/dock behavior on the actual laptop; static analysis cannot establish the impact.

For a helper-related freeze, the first recovery attempt is:

1. If the test is genuinely stuck, hold the power button until the laptop turns fully off.
2. Remove the test USB.
3. Connect the normal Lenovo charger.
4. Start the laptop without the USB.

That should prevent this USB-hosted helper from running again. A fresh boot without it loads the original warning code. The saved startup entry can still remain and may need cleanup afterward. **Removing the USB is the intended escape route, not an absolute guarantee against every firmware problem.** Removing it after the helper has already run does not undo that boot's RAM change; power off first.

If the machine still will not reach the Lenovo logo or BIOS Setup without the USB, stop and report exactly what it does. Do not flash the analyzed BIOS image, clear the TPM, or try random firmware-recovery steps from this package. The analyzed BIOS file was an analysis input, not a recovery image prepared for you to flash.

## 3. Secure Boot and encryption, in plain language

Secure Boot checks whether early-startup programs are trusted. These custom helper files do not carry a signature already trusted by your Lenovo, so they are expected to be blocked while Secure Boot is enabled. Turning Secure Boot off lets us try them. It does not permanently approve them for future boots with Secure Boot back on. [Microsoft's Secure Boot overview](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).

For this procedure, change **Secure Boot = Disabled** only. Keep the factory keys. Do not select “clear keys,” “delete keys,” or reset the TPM. You do not need to enable Legacy/CSM boot.

Changing early-startup settings can also cause BitLocker/device encryption to request a recovery key. That is a drive-access protection event, not proof of firmware damage. Check the current encryption status before changing early-startup settings. [Microsoft explains recovery triggers and recovery information here.](https://learn.microsoft.com/en-us/windows/security/operating-system-security/data-protection/bitlocker/recovery-overview)

To check in Windows:

1. Right-click Start and open **Terminal (Admin)** or **Windows PowerShell (Admin)**.
2. Paste `manage-bde -status C:` and press Enter. This displays status; it does not change encryption.
3. If you see **Fully Decrypted** and **0.0% encrypted**, C: is not encrypted at that point.
4. If it is encrypted, encrypting, suspended, or anything is unclear, establish that you have the recovery key before changing Secure Boot. You can send the status output for interpretation; do not send the recovery key itself.

Microsoft's recovery-key page is [Find your BitLocker recovery key](https://support.microsoft.com/windows/find-your-bitlocker-recovery-key-6b71ad27-0b89-ea08-f143-056f5ab347d6).

Secure Boot being off is a continuing tradeoff during these tests, not just a one-time installation trick. If testing ends, remove the automatic test entry and restore Secure Boot to its original state.

## 4. What the development tests actually establish

Development validation ran on a separate development system. **It did not run on the physical Lenovo.** You do not have to install any test software yourself.

First, I wrote the helper's source code: human-readable instructions in the C programming language. A compiler translated it into the `.efi` programs in the ZIP. Those are programs for the pre-Windows UEFI environment. They are not Windows applications you should double-click.

Then I tested it in two ways.

**Controlled software tests:** I wrote additional code that presents the patch engine with deliberately constructed situations and checks its response. For example: the wrong BIOS code, two matching modules, a page that cannot be written, a permission query that fails, or a restoration operation that reports failure. The expected response in those cases is usually to refuse the patch or report failure without attempting an unsafe follow-up write. There were **33 passing cases**. They are 33 specific checks, not 33 successful trials on your laptop, and they do not establish a numerical brick probability. Extra diagnostic software watched for certain memory-access errors during these tests.

**A virtual computer:** I used QEMU, software that imitates a computer, with OVMF, a UEFI firmware implementation used in virtual machines. This let the actual compiled helper run before an operating system inside a simulated machine. I also tested the Shell installation/removal commands there.

Inside that virtual computer, I supplied an inert copy of the small warning module extracted from the analyzed BIOS as test data. The test checked whether the helper recognized it, changed the expected byte, saved a log, and returned normally. It also presented the “warning trigger already happened” condition and checked that the helper refused a late patch.

I did **not** boot an exact virtual copy of your whole Lenovo, execute its adapter-checking firmware routine, or simulate its actual USB-C charger/controller. The virtual test therefore did not demonstrate a real Lenovo warning disappearing.

One limitation is particularly relevant: this OVMF build lacked the firmware service used to change a read-only code page. The read-only transition and failure handling passed controlled software tests, but that operation was **not exercised through a native firmware implementation in the emulator**. If your Lenovo needs it, the manual laptop test is still required.

These tests are useful for finding mistakes before involving your machine. They cannot reproduce all proprietary Lenovo behavior, hardware interactions, firmware bugs or boot paths. A passing test suite is evidence about the cases tested, not a safety certification.

Source and developer test harnesses are available in the repository. The public USB ZIP contains the USB files and guides; it omits the Lenovo firmware fixture and developer tests. The repository separately includes one reviewed firmware-module fixture for developer tests. You do not need to compile or run developer materials for the laptop walkthrough.

## 5. The overall sequence

| Stage | What you do | What we learn |
|---|---|---|
| 1. Inspect | F12-boot the diagnostic USB with the normal charger. | Does your live firmware match, and what are its memory permissions? |
| 2. Try one temporary patch | F12-boot the manual test with the normal charger. | Can the helper perform and verify the RAM change on your laptop? |
| 3. Test early startup | Register the USB driver, then boot normally with the normal charger. | Does the helper run early enough and leave normal startup working? |
| 4. Test the actual warning | Boot normally using the problem USB-C charger. | Does the message disappear? |
| 5. Compare with/without USB | Fully shut down between boots and remove/reinsert the USB. | Does suppression reliably depend on the helper? |

The normal charger in the first stages keeps the original warning from complicating those checks. The manual test may run too late to prevent the warning on that boot; its purpose is to prove the patch operation. Only the automatic test addresses the required timing.

## 6. Prepare for Stage 1

Have the ZIP, a spare FAT32 USB, the normal Lenovo charger, and your phone available. A phone is useful for photographing the screen and reading this guide while Windows is not running. Back up anything important on the spare USB and save your open work.

Check the firmware version:

1. Press **Windows + R**.
2. Type `msinfo32` and press Enter.
3. Find **BIOS Version/Date**. It should include **GKCN65WW**.
4. If it shows another version, send that information before running the helper. Do not update or downgrade the BIOS just to match this guide.

Check encryption as described above. Then right-click the downloaded ZIP and choose **Extract All**. Open the extracted package.

For the first stage, use only `01_DIAGNOSTIC_USB`. Its contents are a folder named `EFI` and a file named `LenovoWarning-v4-Diagnostic.efi`.

Plug in the spare USB. In File Explorer, open **This PC**, right-click that USB, and select **Properties**. Check that its file system is **FAT32**. If it already is, no formatting is needed. If it is not and Windows does not offer FAT32, stop and identify the USB/capacity before choosing another formatting method. Do not substitute NTFS or exFAT for this test.

Copy the **contents inside** `01_DIAGNOSTIC_USB` to the USB's top level. Do not copy the entire ZIP or the enclosing stage folder as the only folder on the stick.

For example, if the USB happens to be E:, the required files are:

- `E:\EFI\BOOT\BOOTX64.EFI`
- `E:\LenovoWarning-v4-Diagnostic.efi`

E: is only an example; use the letter assigned to your USB. `BOOTX64.EFI` is the same diagnostic under the standard removable-boot filename. These files already include everything needed for Stage 1.

If you previously installed an automatic V2/V3 helper, tell me before collecting a clean result. An older helper could run first and change what the diagnostic sees. The V4 Shell can inspect/remove old test entries, but do not delete an unknown firmware entry yourself.

## 7. Run Stage 1

1. Connect the normal Lenovo charger and leave the prepared USB plugged in.
2. Restart and press **F2** or **Fn+F2** during startup to enter BIOS Setup. The exact screen/menu placement can vary; if the keys do not get you there, report what you see.
3. Find **Secure Boot**. Take a photo of its original setting. Change it to **Disabled**, without clearing keys or changing unrelated settings. Use the firmware's displayed Save and Exit option.
4. At the next startup, press **F12** or **Fn+F12** to open the temporary boot menu.
5. Choose the UEFI entry for the USB. The diagnostic should open directly; there should be no Shell prompt in this stage.
6. Let it finish. It displays a result and attempts to save `LENOVO_V4_DIAGNOSTIC.LOG` to the USB.
7. Photograph the result. Press a key to exit. If it returns to a menu, choose **Windows Boot Manager** to return to Windows. If needed, power off after the diagnostic finishes, remove the USB, and start normally.
8. Open the USB in Windows and review `LENOVO_V4_DIAGNOSTIC.LOG` for personal details, then share its relevant results for review. If the log is missing, upload the photo instead.

**Stop here for the first session.** We need the live result to decide whether Stage 2 is appropriate; you do not need to interpret hexadecimal addresses yourself.

`READY_OR_PATCHED` in this diagnostic means the initial checks found a candidate. It does not mean the diagnostic patched anything. `patched=0` is correct in Stage 1.

If the USB is absent from the boot menu, or you get a security violation, report that screen and the USB layout. Do not turn on Legacy/CSM mode or clear Secure Boot keys to experiment.

## 8. Later: Stage 2, manual patch

Proceed only once the diagnostic has been interpreted and the required conditions are met.

Keep the normal charger connected. In Windows, copy the contents of `02_MANUAL_PATCH_USB` onto the USB, replacing `EFI\BOOT\BOOTX64.EFI`. You do not need to reformat. Retain the earlier log. Safely eject the USB after file copying finishes, then reconnect it for the boot test.

F12-boot the USB again. This time the manual test asks for **uppercase P**. Press **Shift + P** to begin. Another key exits without attempting the patch.

The application checks the target again and attempts the temporary change. It saves `LENOVO_V4_MANUAL.LOG`. Success requires the successful result plus `patched=1`, `verified=1`, and no rollback. If it temporarily cleared read-only protection, restoration must also succeed. Send the log rather than judging success from a single green-looking line or an isolated number.

Fully power off after this test. This separates the manual experiment from the next boot. If anything freezes or reports failure, use the USB-removal recovery sequence and do not install the automatic driver yet.

## 9. Later: Stage 3, automatic startup

This stage is the only helper-installation step that deliberately creates a persistent firmware startup entry. The helper file itself stays on the USB.

Copy the contents of `03_AUTOMATIC_DRIVER_TEST` to the USB. Its `BOOTX64.EFI` is the bundled **UEFI Shell**. The Shell is a text command interface that runs before Windows, similar in appearance to Command Prompt. It is needed to create/remove the early-startup entry; it is not meant to appear on every ordinary boot.

With the normal charger connected and Secure Boot disabled, F12-boot this USB. At `Shell>`, type these one line at a time:

```text
map -r
fs0:
ls
```

The Shell uses `fs0:`, `fs1:`, etc. instead of Windows drive letters. `map -r` refreshes its drive list; `ls` lists files. If you do not see `LenovoWarning-v4-Driver.efi` and `install-driver.nsh`, try `fs1:` followed by `ls`, and continue until you identify the correct USB by those files. Do not assume fs0 is always the USB.

Run:

```text
install-driver.nsh
```

Without the word INSTALL, this only displays the current driver list and instructions. If old Lenovo test entries appear, send a photo so we can identify exactly what to remove. Unrelated entries must be preserved.

Once the prior manual test has passed and the list is clear of competing old Lenovo helpers, run **once**:

```text
install-driver.nsh INSTALL
```

The result should show exactly one entry named **Lenovo USB-C Warning Patch v4 TEST**, and no command errors. Do not run INSTALL repeatedly; it does not automatically prevent duplicate entries. If unsure, photograph the screen before rebooting.

Type `reset`, then let the laptop boot normally **without pressing F12**. Leave the USB inserted in the same port and keep the normal charger connected. The expected visible behavior is ordinary startup to Windows with no helper banner or Shell.

In Windows, upload the new `LENOVO_V4_DRIVER.LOG`. A reported successful patch must also show that the warning trigger was absent just before the patch. This log is what answers the early-startup timing question.

If Windows does not start normally, or there is no new complete log, stop and report the result. A missing log alone does not tell us whether the helper ran, because writing the log is deliberately nonessential to boot.

## 10. Later: use the actual USB-C charger and compare

After the automatic test with the normal charger succeeds:

1. In Windows, copy the driver log somewhere safe and remove the old copy from the USB. This makes the next boot's record unambiguous.
2. Save your work. For a full shutdown, open a terminal and run `shutdown /s /t 0`. This shuts down immediately after applications are handled; do not run it with unsaved work. No forced-close `/f` option is included. [Microsoft shutdown reference](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/shutdown).
3. Keep the USB inserted. Disconnect the normal Lenovo charger and connect the USB-C charger that reliably produces the original warning.
4. Power on normally without F12. Observe whether the warning appears.
5. If it appears, press Esc as usual, let Windows start, and upload the new driver log. Do not interpret the continued warning as a brick.
6. If it does not appear, upload the successful log and check normal USB keyboard/dock operation.

Then perform the comparison across fresh boots:

- USB helper present + same USB-C charger: warning should be absent.
- Fully shut down, remove the USB helper, leave the same charger: warning should return.
- Fully shut down, reinsert the helper USB: warning should be absent again.

This is called an A/B test: compare behavior with and without the proposed cause while keeping the charger the same. It provides much stronger evidence than one apparently successful startup. If the entry disappears through firmware housekeeping when the USB is absent, inspect the driver list before assuming the helper itself failed.

## 11. Remove the automatic setup

Removing the USB prevents that USB-hosted file from being loaded on the next fresh boot. Cleaning up the saved entry afterward avoids leaving a reference to missing media.

F12-boot the Phase 3 USB Shell and run:

```text
bcfg driver dump -v
```

Find **Lenovo USB-C Warning Patch v4 TEST**. Removal uses that entry's **current Option/list position**, not its `Driver####` identifier. Positions can change. The command form is `bcfg driver rm <current-position>`; the angle-bracket text is a placeholder and must not be typed literally.

If the correct entry is unclear, request review of a photo of the list before removing it. Do not blindly type `bcfg driver rm 0`. After removal, display the list again and confirm only the intended entry is gone.

If stopping the project, restore Secure Boot to its original enabled state after removing the unsigned helper's entry. Do not clear or replace the factory keys. Re-enabling it does not allow this unsigned helper to continue suppressing the warning.

## 12. What you need to know right now

You are preparing a USB and running a small pre-Windows diagnostic. You are not being asked to program the BIOS, flash the uploaded file, compile source, or install the automatic patch immediately.

Your first useful deliverable is **LENOVO_V4_DIAGNOSTIC.LOG**, or a photo if logging fails. The later instructions are here so you understand the whole project and can assess the tradeoffs before starting.
