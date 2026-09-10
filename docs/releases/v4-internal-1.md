# V4 internal-drive deployment, revision 1

Move the already-working V4 helper from its USB test location to a dedicated internal EFI folder. Once the internal startup is confirmed, normal boot no longer needs the USB.

The download includes a beginner walkthrough, copy-and-compare Shell script, conservative disable script, unchanged V4 automatic helper, recovery Shell with its license, and checksums. It uses explicit `bcfg` commands to replace only the project's USB driver entry, preserving unrelated entries. The old public USB release remains unchanged.

**Scope:** Lenovo Legion 5 Pro 16ACH6H / 82JQ, GKCN65WW, after a successful reviewed V4 automatic USB test. Warning suppression was demonstrated on the physical laptop, including warning returning without the USB helper and disappearing after reinsertion. The latest automatic log also confirms an early verified patch without a permission change. Normal peripheral checks remain outstanding.

**Deployment status:** new scripts checked in isolated QEMU/OVMF; unchanged helper checked against fixed hashes and 33 host cases. Internal-drive startup on the Lenovo and Windows recovery steps have not yet been physically verified. This prerelease supplies the concrete installation and removal path for that next deployment.

**Secure Boot:** the helper remains unsigned and requires Secure Boot disabled unless a separate trust arrangement is established. The included research note describes possible exact-image approval or certificate enrollment; neither is implemented by this package. There is no BIOS flash or key replacement.

Extract the ZIP, read `READ-ME-FIRST.md`, and copy the contents of `USB` onto the existing recovery/test USB without formatting it. Use that USB for the installation, then remove it after shutting down. Follow the walkthrough rather than registering a path based on an assumed FS number.
