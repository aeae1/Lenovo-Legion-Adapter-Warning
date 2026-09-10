# Provenance and notices

The three V4 helpers and their source are preserved byte for byte from the original project baseline. Their SHA-256 values are recorded in `docs/BINARY-VALIDATION.json`. The public USB download is a separately named, scrubbed package; it is not the original private ZIP.

`03_AUTOMATIC_DRIVER_TEST/EFI/BOOT/BOOTX64.EFI` is the TianoCore UEFI Shell. Its accompanying `TIANOCORE-SHELL-LICENSE.txt` is retained in the repository and public download. Upstream: [Shell binary](https://github.com/tianocore/edk2-archive/blob/master/ShellBinPkg/UefiShell/X64/Shell.efi) and [license](https://github.com/tianocore/edk2-archive/blob/master/ShellBinPkg/License.txt).

No complete Lenovo BIOS image is distributed here. `source/tests/GKCN65WW-warning-fixture.bin` is the single reviewed 23,360-byte AdapterUnsupportWarningPage module from GKCN65WW, included solely as inert test data. Its SHA-256 is `6ea392a3d33a65e326a58e88025aec186daffc71ea7811edd5fce2ca68c00e9f`. It remains third-party firmware material and is not covered by a project open-source license. Do not execute or flash it. It is omitted from the USB download.

Compiler and Python packages are downloaded by the build instructions, not vendored. A project-wide open-source license has not been selected. Third-party notices apply to their respective material; publication does not relicense Lenovo firmware or the bundled Shell.
