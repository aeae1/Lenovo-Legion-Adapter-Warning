# Secure Boot and this helper

**The installed V4 helper currently works with Secure Boot disabled. Offline signing and an isolated Secure Boot VM test now pass. Enabling it on the Lenovo still requires a validated trust-enrollment step and a physical test.**

Secure Boot checks trust in early boot software and helps resist bootkits. Disabling it does not itself disable Windows antivirus or decrypt the disk, but removes this firmware trust check. Restoring it is a useful project goal. See [Microsoft's boot-security explanation](https://learn.microsoft.com/en-us/windows/security/operating-system-security/system-security/secure-the-windows-10-boot-process).

## What signing does, and what it does not

The [signing project](../signing/README.md) can attach and verify a signature on a copy of the exact V4 driver while checking that its original executable bytes are preserved. A new signature alone does not make Lenovo trust the file. The signing certificate must be authorized in the firmware's allowed database, `db`, or the firmware must explicitly approve the image digest. Existing revocations in `dbx` still take precedence. [Microsoft documents these databases](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).

An approval digest uses the UEFI/Authenticode PE-image hashing rules; the ordinary whole-file SHA-256 in our download manifest is not an interchangeable enrollment value. A certificate in Windows' certificate store or a Linux shim/MOK store does not by itself authorize this directly loaded DriverOrder driver.

Signing adds integrity and provenance, not a proof that code is safe. Trusting this helper deliberately authorizes narrow, powerful pre-Windows code that modifies a firmware callback in RAM. No general unsigned-code bypass is part of this project.

## What the Lenovo actually exposes

The [supplied menu photographs](../hardware-results/secure-boot-menu-reviewed.md) show Secure Boot disabled, Platform Mode User Mode, and Secure Boot Mode Standard. They also show Reset to Setup Mode and Restore Factory Keys. They do not show a file-approval or custom-certificate enrollment control.

Static inspection of `SecureBootMgrDxe` found “Select a UEFI file as trusted for execution”, “Add sepecific EFI image hash to allowed database.”, “Add this hash image to allowed database (db)”, and enrollment strings. These indicate relevant implementation code, but do not prove that its UI is reachable on this model.

**Do not use Reset to Setup Mode or Restore Factory Keys as a substitute for adding one trusted file.** The visible help identifies key-reset operations. Their exact effects and a restoration procedure have not been validated for this laptop. No reset/enrollment command is supplied by the signing tools.

## Current evidence and next step

- Offline tests verify a disposable-key signature and reject a wrong certificate and altered code. The frozen original helper is preserved.
- A disposable OVMF machine running SecureBoot=1 and SetupMode=0 accepts the trusted signed helper and rejects unsigned, untrusted, and altered copies. The driver starts without a Lenovo target. This validates signature admission in that VM, not Lenovo enrollment or warning suppression under enforcement.
- The next laptop observation is the [read-only Windows inventory](../signing/README.md#next-step-on-the-laptop-read-only-inventory). It reads state and exports PK/KEK/db/dbx and available default databases to local files. Query failures remain explicit. Exports are not a guaranteed recovery package.

Before any future enrollment, establish a specific supported way to add only the intended trust record, preserve existing OEM/Microsoft trust and revocations, and recover from failure. If temporary Setup Mode is ever considered, it needs a separately reviewed preservation/restoration procedure; it is not today's instruction.

Still unproven: Lenovo enrollment access, retention of existing keys, early DriverOrder timing and writable target memory with Secure Boot enabled, encryption/recovery behavior, and disabling a now-trusted helper. Enabling Secure Boot would no longer block a helper that has successfully been trusted, so the existing unsigned-helper recovery shortcut would need revision.

Keep the working installation and Secure Boot setting unchanged while this research proceeds. The original internal-deployment ZIP retains its earlier research note; [release snapshots](releases/v4-internal-1-snapshot/) preserve those published bytes.
