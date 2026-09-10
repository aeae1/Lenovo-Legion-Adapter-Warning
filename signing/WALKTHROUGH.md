# Secure Boot signing: walkthrough and risks

For the Lenovo Legion 5 Pro 16ACH6H / 82JQ, BIOS GKCN65WW. Prepared September 10, 2026.

**You can begin with the Windows inventory below. It requires no reboot and makes no firmware changes. The exact Lenovo enrollment procedure is not established yet, so the later stages explain the intended sequence, not commands to execute today.**

Version 1.0.0 is the regular release of the working, unsigned installation. Its helper is the same V4 executable already tested internally. An existing working installation needs no upgrade. Secure Boot support remains a separate development milestone.

## What we are trying to accomplish

Keep the warning-free internal startup, then restore Secure Boot so the firmware checks authorization of externally loaded boot software. This takes two separate operations:

1. **Sign a copy of the helper.** This creates a verifiable signature using a private key. It changes an ordinary output file, not the laptop's firmware settings.
2. **Authorize that signature on the Lenovo.** The firmware must trust the signer, or explicitly approve the particular image. This changes persistent firmware trust and is the more consequential part.

A self-signed certificate is not automatically trusted by Lenovo or Microsoft. Putting it in the Windows certificate store does not authorize a directly loaded UEFI DriverOrder driver. The already passing tests establish offline signing and admission in OVMF, not a working Lenovo enrollment procedure.

[Microsoft's Secure Boot overview](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot) describes how firmware checks UEFI drivers and uses its trust databases.

## What is already proven

| Check | Result and limits |
| --- | --- |
| Existing unsigned helper on the Lenovo | Warning suppression reported from USB and internal SSD; basic mouse and storage operation reported |
| Original helper verification | Byte-identical rebuilds and all 33 host fault-injection cases passed |
| Offline signing | Signature verifies; wrong certificate and modified code fail; original executable payload preserved |
| Disposable Secure Boot VM | Trusted signed driver accepted and started; unsigned, untrusted, and modified copies rejected |
| Windows inventory script | Windows PowerShell 5.1 CI passed with mocked firmware reads |
| This Lenovo with Secure Boot enabled | Not tested; enrollment and early helper operation remain unresolved |

No permanent signing key has been made for the owner. Disposable test keys must never become the laptop's trust keys.

## Stage 1 — do this now, in Windows

You do not need the UEFI Shell, a new USB, a BIOS visit, or another warning comparison for this stage. Keep your existing working setup as it is.

### 1. Download the small inventory package

Open the [Version 1.0.0 release](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/releases/tag/v1.0.0) and download **Lenovo-Secure-Boot-Inventory-v1.0.0.zip** from Assets. This is the small preparation package; the full installer is not needed on an already working machine.

Right-click the ZIP, choose **Extract All**, and open the extracted folder. Run files from the extracted folder, not from inside the ZIP. The package contains this guide, `START-HERE.txt`, two inventory scripts, and a checksum manifest. It contains no EFI executable, enrollment program, or signing private key.

### 2. Check drive encryption

Right-click Start and open **Terminal (Admin)** or **Windows PowerShell (Admin)**. Approve the Windows administrator prompt, then run:

```powershell
manage-bde -status C:
```

This command reads status. It does not turn encryption on or off.

- If it says **Fully Decrypted** and **0.0% encrypted**, that volume is not encrypted at that time.
- If it is encrypted, encrypting, suspended, or unclear, establish that you have the correct recovery key stored separately before any later boot-setting change. You can still perform today's read-only inventory.
- Tell the reviewer the conversion status, percentage encrypted, and protection status. Do not send the recovery key or run a key-display command for sharing.

Boot changes can trigger BitLocker recovery. Without the necessary recovery information, encrypted data can become inaccessible. Suspending protection for a later planned change is different from decrypting a disk; any suspension/resumption will be scheduled only with that actual change. [Microsoft's recovery overview](https://learn.microsoft.com/en-us/windows/security/operating-system-security/data-protection/bitlocker/recovery-overview) explains the triggers and recovery options.

### 3. Run the inventory

In the extracted folder, right-click **Read-SecureBootState.cmd** and choose **Run as administrator**. On Windows 11 this may be under **Show more options**. Approve the normal Windows administrator prompt.

The launcher runs the accompanying PowerShell script with an execution-policy exception for that process only. It does not permanently change the machine's execution policy. Administrator access lets the documented Windows cmdlets read the firmware variables. If Windows or security software blocks it, keep the exact message for review; do not disable system security tools to force it through.

Expected final message:

```text
Finished reading Secure Boot state. No firmware settings were changed.
```

The script reads `SecureBoot`, `SetupMode`, `PK`, `KEK`, `db`, `dbx`, and available default databases, then saves ordinary local files. It does not mount the EFI partition, replace the driver, change DriverOrder, enroll a certificate, clear keys, or enable Secure Boot. [Microsoft documents these read operations and their administrator requirement](https://learn.microsoft.com/en-us/powershell/module/secureboot/get-securebootuefi?view=windowsserver2025-ps).

### 4. Find and send the result

Beside the scripts, open **private-results**, then the newly created **SecureBoot-...** folder.

Send **summary.json** in this conversation, plus the encryption-status fields from step 2. Keep the `.bin` exports locally. Do not post the whole results folder to a public GitHub issue. If the script failed, send the visible error instead; do not guess at a firmware fix.

The summary contains state flags, database sizes, attributes, hashes, and read failures. It does not collect your computer name, serial number, Windows username, or recovery key. The database exports contain public certificates/hashes, not their private signing keys, but reveal the machine's trust configuration.

**Stop here for review.** This checkpoint exists because the laptop-specific enrollment route and recovery plan need real device information. It is not a request to repeat the successful installation tests or obtain permission for ordinary signing preparation.

### 5. How the result will be used

The reported setup suggests `SecureBoot=0` and `SetupMode=0` with existing platform keys, but those values must come from the actual inventory. Secure Boot being off does not necessarily mean the databases are freely writable. Missing optional defaults are different from failure to read a required active database.

Hashes and sizes show what was read; they do not identify every certificate or prove an enrollment menu exists. If parsing a specific database is necessary, the reviewer may request that particular export privately. The summary alone is not a certificate inventory.

## Stage 2 — establish a supported trust route

The supplied BIOS photographs show **Standard**, **User Mode**, **Reset to Setup Mode**, and **Restore Factory Keys**. They do not show a working certificate-addition or image-approval control. Relevant strings in the firmware are clues, not proof of an accessible menu.

The preferred route would preserve the current OEM/Microsoft records and revocations and add only the intended authorization:

| Route, if supported | Consequence |
| --- | --- |
| Approve one exact EFI image | Narrow trust for that image; changing the image can require new approval |
| Add a dedicated signing certificate to `db` | Allows qualifying images signed by that key, subject to other policy/revocation checks; protects future updates only while that private key remains controlled |

An image approval uses the firmware's PE/Authenticode image-hash rules. The ordinary whole-file SHA-256 in the download manifest is not a replacement enrollment value.

**Do not select Reset to Setup Mode, clear PK, or Restore Factory Keys as a shortcut.** If the only route requires replacing platform ownership or rebuilding the trust databases, that is a materially different procedure requiring a specific preservation and restoration plan. It is not necessary merely to create a signature. Retain the current trust state, including recent updates; factory defaults are not necessarily the current databases.

For orientation: `db` holds allowed signers/images, `dbx` holds revocations, `KEK` authorizes database updates, and `PK` is the platform ownership key. Their exports are useful records, not a tested one-click restore package. See [Microsoft's database explanation](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).

## Stage 3 — create the signing identity and sign an offline copy

After selecting the trust route, create the owner's permanent private key on an owner-controlled machine, outside the repository. A public certificate can be shared for verification or enrollment; the private key must stay private and have a secure backup. Do not put it in GitHub, chat, or the setup USB.

The repository already has an offline Linux signer, `signing/sign-helper.py`. It takes an existing private key and matching public certificate and writes into a new output folder. It checks the frozen input hash, verifies the result, and rejects unexpected changes to the original executable content. The developer commands are in [the signing README](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/blob/main/signing/README.md).

The resulting signed helper has a different whole-file hash. It needs its own checksum and reviewed deployment instructions. Signing is not Microsoft approval, a warranty, or a general malware-safety assessment. No bought certificate is required simply to conduct the personal-trust approach described here; it still needs an accessible Lenovo enrollment method.

A lost key does not erase an existing signature, but prevents signing new files under that identity. A stolen key can authorize other code under that certificate on devices that trust it. A certificate for this helper should not be reused as a general-purpose signer.

There is no reason to install Linux or WSL for today's inventory. Once the trust route is known, choose a suitable owner-controlled signing environment and provide exact commands for that environment.

## Stage 4 — prepare recovery, then enroll and deploy

Before persistent changes, review all of these against the actual laptop:

- Current trust exports and any applicable encryption recovery key are available separately.
- The original unsigned helper remains recoverable, with its known hash.
- A way to disable the project file or exact driver entry remains reachable if a trusted helper hangs before Windows.
- Any required recovery USB can be booted under the intended security setting, or there is a verified way to change that setting and reach recovery.
- The exact enrollment action, certificate/image, destination database, and checks for retained existing trust are established.

This matters because **enabling Secure Boot stops being an escape hatch once the helper itself is trusted**. The existing unsigned Shell is also blocked under normal Secure Boot enforcement. Disabling Secure Boot would allow that Shell again if firmware settings remain accessible, but it would not stop a trusted or unsigned helper from running earlier. The recovery sequence must account for both facts.

Only then use the reviewed enrollment and deployment steps, preserving the original file until the new copy and recovery route have been verified. No automatic installer in Version 1 performs this stage. No database-write commands are provided here because the required Lenovo interface has not been established.

## Stage 5 — verify the finished arrangement

After the planned changes, test normal startup and USB-C warning suppression, then verify **Secure Boot State: On** in Windows System Information (`msinfo32`) or the documented `Confirm-SecureBootUEFI` query. Check normal mouse/storage operation and the actual new helper result. A BIOS toggle alone is not the complete validation.

Retain existing Windows/OEM trust and revocations, confirm encryption protection is resumed if it was temporarily suspended, and record which exact signed helper and certificate are in use. A successful OVMF run cannot substitute for these physical results.

## Risks, without pretending there is a measured percentage

| Action or failure | Practical risk |
| --- | --- |
| Today's inventory | Standard firmware reads and local report files; no intended boot change. A read error should stop the script or appear in its report. |
| Signing a separate file offline | Does not change the installed setup. Main concerns are key handling and choosing/verifying the correct file. |
| Enabling Secure Boot before the helper is trusted | Helper can be rejected and the original warning can return; firmware handling of a rejected driver is device-specific. |
| Incorrect trust enrollment or key replacement | Can block Windows or recovery media. This is the part with meaningful startup and recovery consequences. |
| Boot changes on an encrypted installation | Can trigger a recovery-key prompt. Loss of the necessary key can mean loss of access to encrypted data. |
| Wrong EFI file operation or interrupted write | Can damage startup files. Use a separate project location and a verified recovery procedure. |
| Trusted helper hangs before Windows | An internal driver may run before the boot menu. The current unsigned-helper recovery shortcut no longer suffices after trust enrollment. |
| Helper's existing RAM patch | Signing does not reduce its code risk or establish wider hardware compatibility. The skipped callback includes a USB connection call; basic devices passed, broader behavior is untested. |

This work does not flash the BIOS executable image or alter charger wattage. That removes the specific risk of a failed BIOS-image flash, but does not make persistent trust or EFI filesystem changes risk-free. Startup failures may be recoverable if settings or recovery media remain reachable; that is not a guarantee. No defensible numeric brick probability has been measured for this Lenovo.

For the owner already running internally, the useful next action is Stage 1: send `summary.json` and the encryption-status fields. The rest of this guide explains what must be established before turning Secure Boot back on.
