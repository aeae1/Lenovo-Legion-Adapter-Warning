# Secure Boot and this helper

Secure Boot checks whether early boot software is trusted before allowing it to run. Its practical value is protection against bootkits that start before Windows and can interfere with later operating-system defenses. Turning it off does not itself turn off Windows antivirus or decrypt the disk, but it removes this firmware-enforced trust check. We prefer restoring it for the finished setup if a reliable enrollment route is available. See [Microsoft's boot-security explanation](https://learn.microsoft.com/en-us/windows/security/operating-system-security/system-security/secure-the-windows-10-boot-process).

The current V4 helper is unsigned. It has physically suppressed the warning with Secure Boot disabled. Moving the same file from USB to the internal EFI partition does not change its trust status.

## Two possible personal-device trust routes

1. **Approve this exact EFI image.** Firmware can store an allowed image digest in its `db` database. Where a firmware menu supports selecting an EFI file as trusted, it can calculate and enroll the appropriate image digest. This can preserve the exact existing binary and avoid managing a signing key. A changed executable may require new approval.
2. **Sign the helper and enroll our certificate.** Signing is technically feasible, but the Lenovo firmware must trust that certificate in `db`. A signature from a newly created key is not automatically trusted. The private signing key must remain private, off the public repository and installation USB. Preserve the existing factory Windows/OEM trust entries and revocations.

Microsoft documents both signer certificates and image hashes in the firmware's allowed database, including their use for UEFI drivers. Revocations in `dbx` take precedence. See [Secure Boot databases and keys](https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/oem-secure-boot).

The Secure Boot image digest is the UEFI/Authenticode PE digest, not necessarily the whole-file SHA-256 used by this project's download manifest. Do not paste the ordinary archive/helper checksum into an enrollment tool as though they were interchangeable. No enrollment artifact or signing key is supplied in the internal-deployment package.

Approving this helper deliberately trusts powerful pre-OS code that modifies a narrow firmware callback in memory. That is a specific addition to the trust set; the plan is not to install a general unrestricted unsigned-code loader. Signing is provenance and integrity, not a proof of safe behavior.

## Evidence in the supplied GKCN65WW firmware

Static inspection of the extracted `SecureBootMgrDxe` module found these strings:

- `Select a UEFI file as trusted for execution`
- `Add sepecific EFI image hash to allowed database.`
- `Add this hash image to allowed database (db)`
- `Enroll KEK Using File`, `Enroll Signature`, and related management strings.

These are clues to an existing image-approval/key-management implementation. They do **not** prove Lenovo exposes the corresponding menus on this model. The extracted SetupUtility security form also contains Secure Boot controls, status text, reset-to-setup-mode, and restore-factory-keys actions. Those destructive/reset actions are not equivalent to approving one file and should not be used as a substitute.

The next observation is the actual BIOS Secure Boot screen and any visible trust/custom-key menus, with no keys cleared or settings changed merely for discovery. If only the enable/disable and reset options are exposed, further implementation research is needed before proposing key enrollment.

## What remains unproven

- Whether this laptop exposes an accessible file-approval or certificate-enrollment path.
- Whether enrollment can add only the intended trust record while retaining the working factory configuration.
- Whether the same driver starts early enough and finds writable target memory with Secure Boot enforcement enabled. A successful signature check would establish load permission, not automatically establish the whole patch path.
- Windows encryption/recovery behavior after a trust configuration change.

For this directly loaded DriverOrder driver, adding a certificate only to Windows' certificate store or to a Linux shim/MOK database would not by itself establish trust in the firmware's `db`. No Microsoft-wide signing approval is claimed or required for the proposed personal-device routes.

Internal-drive deployment can be prepared independently. Keep the currently working Secure Boot setting until a concrete trust configuration and recovery plan have been reviewed.
