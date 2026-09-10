# Lenovo adapter-warning test — V4

**Start with the read-only diagnostic. This is a test package, not a confirmed fix for your laptop.**

Target: Lenovo Legion 5 Pro 16ACH6H / 82JQ, BIOS **GKCN65WW**.

1. In Windows, check `msinfo32`: BIOS Version/Date should include **GKCN65WW**. Use the normal Lenovo Lenovo charger for the first two phases.
2. Check current drive-encryption status with `manage-bde -status C:` in an administrator terminal. If BitLocker/device encryption is active, have the working recovery key available before changing Secure Boot. Do not send the recovery key with the diagnostic.
3. Use a spare **FAT32** USB. Copy the **contents of `01_DIAGNOSTIC_USB`** to its root. You should have `EFI\BOOT\BOOTX64.EFI`. No extra download is needed.
4. Enter BIOS with F2/Fn+F2 and set **Secure Boot = Disabled**. Keep the factory keys; do not clear or replace them.
5. F12-boot the USB. The diagnostic opens directly. It only inspects firmware and saves a text log to that USB.
6. Photograph the result if needed, then press a key to exit. Return to Windows and **send `LENOVO_V4_DIAGNOSTIC.LOG` from the USB**. If the log cannot be written, send the screen photo.

The next phases and their files are already included in `WALKTHROUGH.md`: a manual RAM test, then a USB-hosted automatic driver test and removal instructions. Review the live diagnostic before proceeding to the manual test.

If you already have an older automatic Lenovo test driver registered, remove its entry using the Shell in `03_AUTOMATIC_DRIVER_TEST` and `REMOVE-DRIVER.txt` before collecting a clean diagnostic. Merely replacing old USB files does not remove a DriverOrder entry.

**Nothing here flashes the BIOS or changes charger identification.** The patch version changes one byte of the loaded warning callback in RAM; automatic installation adds a standard `Driver####`/`DriverOrder` entry pointing to the USB. It needs to run every boot. These EFI binaries are unsigned, so re-enabling Secure Boot would block them unless a separate supported trust arrangement is established.

If a test hangs, fully power off and remove the USB before booting again. There is no internal-disk installation in this package.

`docs/COMPARISON-WITH-V3.md` explains what was recovered from last time, the source defects corrected, and what is actually new. `docs/VALIDATION.md` records the test evidence and remaining limits.

Reference for the encryption check: [Microsoft's BitLocker recovery overview](https://learn.microsoft.com/en-us/windows/security/operating-system-security/data-protection/bitlocker/recovery-overview).
