# Internal deployment revision 1 validation

The V4 helper is unchanged. Its USB warning-suppression result is physical evidence recorded separately in `hardware-results/`. This record covers only the new deployment scripts and packaging.

## Local isolated Shell result

QEMU 8.2.2 with Ubuntu OVMF 2024.02 and the same TianoCore Shell used on the laptop exercised the new scripts with two disposable FAT32 disk-image files. No host disk, real Windows installation, or Lenovo firmware image was attached. An inert text file stood in for the expected Windows Boot Manager path; it was never executed.

The checks covered:

- Missing source and wrong destination refusal.
- No-argument help and destination-check mode without creating project files.
- Fresh project-folder creation, helper copy and successful byte comparison.
- A deliberately different pair of files returning a comparison error, validating the script's use of the Shell comparison status.
- Refusal to overwrite an existing project folder.
- Migration from a VM USB TEST entry to an internal-path INTERNAL entry, preserving an unrelated VM driver entry.
- Disable check-only mode leaving the active helper present.
- Rename-based disabling and refusal when no active helper remains.
- Removal of only the VM project entry, with the unrelated entry still present.
- The final disabled file's SHA-256 matching the original V4 helper and the inert Windows path marker remaining byte-identical.

The first emulator setup did not connect the second disk; its missing-target checks refused as designed. The harness now explicitly connects the virtual controllers before mapping. A diagnostic echo containing a bare option token was also corrected before packaging. Only the final run with the stated assertions is counted as passing.

Reproduce on Ubuntu with `qemu-system-x86`, `ovmf`, `mtools` and `dosfstools` installed:

```bash
python3 scripts/check-internal-shell.py
```

`V4_QEMU_PREFIX` can point to an extracted Ubuntu package root. The script creates its disposable images under `build/internal-shell-check`. It does not pass any host block device to QEMU.

## Existing binary checks and release gates

`scripts/check.sh` continues to require byte-identical V4 helpers and the 33 sanitized host cases. The original public ZIP remains fixed. `scripts/package-internal.py --check` verifies the eight explicitly selected new archive members, their source bytes, manifest and archive checksum.

The internal prerelease workflow requires both the existing V4 checks and a fresh isolated Shell run before publishing. No existing release asset is replaced.

## Limits

These tests do not establish Lenovo internal-drive discovery/timing, Secure Boot enrollment or enforcement, actual warning suppression from the internal location, USB peripheral behavior, Windows volume mounting, encryption recovery, or the suggested BIOS recovery path. Those claims must follow actual laptop observations. The first normal startup without the helper USB remains the deployment confirmation to collect.
