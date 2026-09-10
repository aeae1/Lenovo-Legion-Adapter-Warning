# Internal deployment and peripheral observations

**Reviewed September 10, 2026. Model: Lenovo Legion 5 Pro 16ACH6H / 82JQ. BIOS: GKCN65WW.**

## Evidence supplied

- UEFI Shell photographs showed the internal Windows EFI volume identified on an NVMe device and the USB as a separate volume.
- `stage-internal.nsh` first checked the destination, then copied with `COPY`. The comparison reported no differences and the script ended with `STAGED_OK`.
- A later photograph showed the USB TEST driver entry removed and exactly one project entry named **Lenovo USB-C Warning Patch v4 INTERNAL**, pointing to `\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi` on internal NVMe storage. No command error was visible.
- The owner then reported normal startup with USB-C power, no helper thumb drive, and no F12 selection. In the context of the completed warning-suppression comparison, this was the successful internal deployment report.
- The owner subsequently reported that the mouse and thumb drive worked.

The helper copied was the existing V4 automatic driver, whose published SHA-256 is `665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898`. The on-device copy comparison is visible; a separate physical hash measurement was not supplied. Secure Boot remained disabled.

## Conclusion and limits

The reported normal internal startup removes the everyday dependency on the helper USB in this tested setup. Mouse and storage observations provide the requested basic peripheral check after skipping the callback that also contains a USB controller connection call.

No full internal-run driver log was supplied. Do not copy values from the earlier USB log and present them as internal measurements. Dock behavior, broader USB initialization, long-term reliability, other BIOS versions, and physical internal disable/recovery remain untested. The setup USB should be kept for recovery.

The original photographs and raw device paths are not published because they contain machine/partition identifiers. This record preserves the relevant observations without those identifiers.
