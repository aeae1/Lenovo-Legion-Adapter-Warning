# V4 public USB test — packaging revision 1

**Experimental. GKCN65WW on Legion 5 Pro 16ACH6H / 82JQ only. No successful physical-laptop result is recorded.**

This public package preserves the exact V4 diagnostic, manual, and automatic helper binaries. The ZIP has a new name and checksum because the extracted Lenovo firmware test module, developer materials, and personal context are omitted. The original private ZIP is not distributed here. An existing V4 Phase 1 USB needs no update.

Download `Lenovo-GKCN65WW-V4-Public-USB-Test.zip`, extract it, and read the beginner walkthrough or START-HERE.md. Start with `01_DIAGNOSTIC_USB` and the normal Lenovo charger. Review Phase 1 before proceeding. The bundled TianoCore Shell includes its license.

The diagnostic inspects firmware and writes a USB log. Patch stages attempt a one-byte RAM change for that boot. Automatic installation adds a standard DriverOrder entry; the package does not flash the BIOS. These helpers are unsigned; re-enabling Secure Boot does not automatically trust them.

Public checks verify the package, byte-identical builds, and 33 sanitized host cases. The repository includes the single reviewed inert fixture for those tests; the USB download omits it. Historical synthetic OVMF results are documented separately. Physical permissions, automatic timing, warning suppression, and USB behavior remain open. Phase 1 alone cannot settle all of them.

The release checksum file identifies the public ZIP. Inside the archive, `python3 verify-package.py` checks its individual files.
