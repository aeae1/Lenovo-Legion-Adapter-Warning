# Deploy the working V4 helper internally and verify removal

The automatic USB test, warning/no-helper comparison, and latest automatic log review are complete. The owner requested startup without leaving the USB inserted. Normal peripheral observations remain outstanding and can be collected at the next startup.

Revision 1 is prepared in `deployment/internal-1/`: a new dedicated internal EFI folder, a conservative copy/readback Shell script, explicit `bcfg` migration from USB TEST to INTERNAL, a disable-by-renaming script, and beginner installation/removal/recovery instructions. The helper bytes and original public USB release are unchanged. A separately named archive includes the recovery Shell and license.

The new scripts have isolated QEMU/OVMF checks for path guards, check-only behavior, copy/readback, refusal to overwrite an existing folder, disabling, entry migration/removal and unrelated-entry preservation. This does not establish physical internal-drive timing or Windows recovery.

The first internal startup still needs physical confirmation without the USB. Keep Secure Boot at the currently working disabled setting for installation; the preference is to restore it after an image-approval or signing/enrollment route is established. See `docs/SECURE-BOOT-OPTIONS.md`. Internal deployment does not itself establish trust.

Completion: publish the concrete package, review the first internal warning-free startup and normal peripheral observations, and retain documented disable/removal paths. Do not claim the internal deployment has already run on the laptop.
