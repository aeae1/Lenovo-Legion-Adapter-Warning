# Secure Boot signing project

**Status: offline signing and Secure Boot admission in a disposable VM validated; Lenovo trust enrollment and Secure Boot-enabled warning suppression not yet tested.** The working internal installation stays on the original unsigned V4 driver. Version 1.0.0 uses that same executable; there is no signed driver upgrade to install.

**[Detailed owner walkthrough and risks](WALKTHROUGH.md)** — start here for the complete sequence and the Windows-only first step.

Signing is feasible. The harder device-specific step is teaching the Lenovo firmware to trust that signature while retaining its existing Windows/OEM keys and revocations. The [Secure Boot note](../docs/SECURE-BOOT-OPTIONS.md) explains the two possible routes: certificate enrollment or exact-image approval.

## Next step on the laptop: read-only inventory

This runs in **Windows**, not the UEFI Shell. It reads settings and writes ordinary local result files. It does not change firmware keys, mount the EFI partition, replace the installed driver, or enable Secure Boot.

1. Open the [Version 1.0.0 release](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/releases/tag/v1.0.0), download **Lenovo-Secure-Boot-Inventory-v1.0.0.zip**, and extract it fully. Do not run from inside the ZIP.
2. Open the extracted inventory folder. If using a full source checkout instead, open its `signing` folder.
3. Right-click **Read-SecureBootState.cmd** and choose **Run as administrator**. Approve the Windows administrator prompt. Administrator access is required by Microsoft's firmware query cmdlets. The launcher permits this script for that PowerShell process only; it does not persist an execution-policy change.
4. It should say **Finished reading Secure Boot state. No firmware settings were changed.** If it reports an error, keep the message for review instead of changing BIOS settings.
5. In `private-results` beside the scripts, open the newly created `SecureBoot-...` folder. Share **summary.json** privately for review. Keep the `.bin` exports locally; do not post the entire folder to a public issue.

The summary contains state flags, variable sizes, attributes, hashes, and any query failures. It does not collect the computer name, serial number, Windows username, or encryption recovery key. A missing optional default variable can be normal; a missing required database still needs review. Each run creates its own folder.

These exports contain public trust material, not private signing keys, but can reveal a device's trust configuration. They are **inventory files, not a tested one-click recovery backup**. Do not clear PK, reset keys, or re-enable Secure Boot based only on their existence.

## Tools

| File | Purpose |
| --- | --- |
| `Read-SecureBootState.cmd` / `.ps1` | Read-only Windows state and trust-database inventory |
| `sign-helper.py` | Sign a copy of the frozen driver using an existing private key and matching certificate |
| `check-signing.py` | Disposable-key signature, tampering, payload, and failure-cleanup checks |
| `check-inventory.ps1` | Windows PowerShell 5.1 tests with mocked firmware queries |
| `check-uefi-load.py` / `vm-load-check.c` | Disposable OVMF Secure Boot admission test |

## Offline signing, for developers

On Linux, install `sbsigntool` and Python `pefile==2024.8.26`. With an existing personal RSA PEM key and X.509 PEM certificate outside the repository:

```bash
.venv/bin/python signing/sign-helper.py   --key /private/location/personal.key   --cert /private/location/personal.pem   --out-dir /private/location/new-signed-output
```

The output directory must be new. Inside the checkout, only a subdirectory of ignored `build/` is permitted. The tool checks the frozen input hash, signs a copy, verifies it with the supplied certificate, and compares the original payload. Only the PE checksum and certificate-directory fields may differ within the original image; the certificate is appended. It refuses an existing output directory and removes a failed output EFI file. It never enrolls, installs, or modifies the source driver.

The signed file has a different whole-file hash. A future signed deployment would require its own reviewed package, checksums, trust configuration, and recovery instructions. Do not overwrite the working internal helper with a test copy.

Private keys must stay off GitHub and the installation USB. No permanent signing key is supplied. All keys made by the tests are disposable and deleted afterward; do not enroll their certificates on the laptop.

## Developer validation

| Check | Evidence / limit |
| --- | --- |
| Offline signing | Locally passed: correct signature verifies; wrong cert and tampered code rejected; original payload preserved; existing output refused; mismatched-key output removed |
| OVMF Secure Boot admission | Locally passed: SecureBoot=1, SetupMode=0; trusted helper accepted and started; unsigned, untrusted, and tampered helpers rejected with EFI_ACCESS_DENIED |
| Windows inventory | CI parses and exercises it with mocked Windows PowerShell 5.1 queries, including missing variables, separate reports, and unsupported firmware. See the actual Actions result; no physical Lenovo inventory yet |
| Original V4 checks | Unchanged 33 sanitized host cases, fixed helper hashes, and archive checks remain separate |
| Physical Lenovo with Secure Boot enabled | Not tested; no trust enrollment performed |

Local Linux checks:

```bash
sudo apt-get install sbsigntool qemu-system-x86 ovmf mtools dosfstools
.venv/bin/python -m pip install pefile==2024.8.26 ziglang==0.13.0 virt-firmware==26.9
mkdir -p build
.venv/bin/python signing/check-signing.py
.venv/bin/python signing/check-uefi-load.py
```

The VM uses newly created disk/variable images under ignored `build/`, no host disk attachment, and no network. A small trusted test application calls UEFI `LoadImage` on four driver copies; it calls `StartImage` only on the accepted trusted copy. Denied images are not forced to execute. No Lenovo firmware target is installed, so the helper finds no patch candidate. Test firmware trust is created only in the disposable OVMF variables file.

In plain language: these are temporary pretend computers and deliberately broken test inputs. They catch packaging, signing, and error-handling mistakes here before any laptop change. They do not simulate Lenovo's charger or establish that enrollment will work on your firmware.

Sources: [Microsoft Secure Boot databases](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot), [Get-SecureBootUEFI](https://learn.microsoft.com/en-us/powershell/module/secureboot/get-securebootuefi?view=windowsserver2025-ps), [Confirm-SecureBootUEFI](https://learn.microsoft.com/en-us/powershell/module/secureboot/confirm-securebootuefi?view=windowsserver2025-ps), and [virt-firmware](https://gitlab.com/kraxel/virt-firmware).
