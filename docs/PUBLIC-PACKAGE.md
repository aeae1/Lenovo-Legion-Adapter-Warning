# V4 public package, revision 1

`Lenovo-GKCN65WW-V4-Public-USB-Test.zip` is a public redistribution of the V4 USB test files and guides. The diagnostic, manual helper, automatic helper, and bundled TianoCore Shell are byte-identical to the original baseline. A Phase 1 USB already prepared from V4 does not need updating.

The archive is deliberately different from the original private ZIP. It omits the extracted Lenovo firmware test module, developer test materials, and personal conversation context. Source, test harness code, and one reviewed inert firmware fixture remain available in the repository. The public filename and SHA-256 identify this packaging revision; they must not be confused with the original private ZIP.

The included `SHA256SUMS.txt` covers every download file except itself. Run `python3 verify-package.py` after extraction for a read-only integrity check. The release also provides a checksum for the entire ZIP.

The USB download contains no complete BIOS image, extracted Lenovo module, personal device log, or private git history. The repository separately contains one reviewed warning-module fixture to keep the host tests reproducible. Research retains the supported model/BIOS version, firmware hashes, protocol GUIDs, offsets, and short matching signatures. These are compatibility evidence, not personal device identifiers.

**Experimental:** no successful physical-laptop test is recorded. Start with the diagnostic and normal Lenovo charger. Read the beginner walkthrough before changing boot settings or running a patch stage.
