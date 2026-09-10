# V4 public USB test — packaging revision 1

**Experimental. GKCN65WW on Legion 5 Pro 16ACH6H / 82JQ only. Physical warning suppression and subsequent internal-drive startup are now demonstrated on the tested machine.**

This public package preserves the exact V4 diagnostic, manual, and automatic helper binaries. The ZIP has a new name and checksum because the extracted Lenovo firmware test module, developer materials, and personal context are omitted. The original private ZIP is not distributed here. An existing V4 Phase 1 USB needs no update.

Download `Lenovo-GKCN65WW-V4-Public-USB-Test.zip`, extract it, and read the beginner walkthrough or START-HERE.md. Start with `01_DIAGNOSTIC_USB` and the normal Lenovo charger. Review Phase 1 before proceeding. The bundled TianoCore Shell includes its license.

The diagnostic inspects firmware and writes a USB log. Patch stages attempt a one-byte RAM change for that boot. Automatic installation adds a standard DriverOrder entry; the package does not flash the BIOS. These helpers are unsigned; re-enabling Secure Boot does not automatically trust them.

Public checks verify the package, byte-identical builds, and 33 sanitized host cases. The repository includes the single reviewed inert fixture for those tests; the USB download omits it. Historical synthetic OVMF results are documented separately. Physical logs confirmed writable target memory and early automatic patching. The owner reported warning absent/present/absent with/without/with the helper USB, then normal internal-drive startup without it, with mouse and storage working. These later observations do not establish other machines, docks, long-term behavior, or Secure Boot-enabled operation. Phase 1 alone cannot settle automatic timing.

The release checksum file identifies the public ZIP. Inside the archive, `python3 verify-package.py` checks its individual files.

**Documentation update — September 10, 2026:** release assets and checksums are unchanged. Text inside the ZIP preserves its original publication checkpoint; use the [current README](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning#readme) for later results and the [signing project](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/tree/main/signing) for Secure Boot research. Offline signing and disposable-VM enforcement are validated; Lenovo enrollment is not yet established.
