# Source and tests

The three shipped EFI binaries are built from `main.c` and the shared headers. `MODE=0` is a diagnostic application, `MODE=1` a manual application, and `MODE=2` an automatic boot-services driver. There is no firmware-variable write routine in these programs. Only the explicitly run Shell installation/removal commands edit DriverOrder.

Build on x64 Linux with Python 3:

```bash
python3 -m pip install ziglang==0.13.0
bash build.sh
```

The bundled Zig/Clang/LLD toolchain produces PE32+ x64 files with EFI subsystem 10 (applications) or 11 (driver), no OS imports and a base-relocation directory. `normalize_pe.py` clears the link timestamp/checksum. Rebuilding with this version was verified to produce identical binaries.

Host fault injection, using GCC with AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
gcc -std=c11 -O1 -g -fshort-wchar -fsanitize=address,undefined \
  -fno-omit-frame-pointer tests/host.c -o host-tests
ASAN_OPTIONS=detect_leaks=0 ./host-tests tests/GKCN65WW-warning-fixture.bin
```

LeakSanitizer is disabled because the development container cannot support its process-inspection mechanism; bounds and undefined-behavior checks remain enabled. The fixture is the reviewed AdapterUnsupportWarningPage module from GKCN65WW, used only as inert test data. Its SHA-256 is `6ea392a3d33a65e326a58e88025aec186daffc71ea7811edd5fce2ca68c00e9f`. It contains generic firmware code, strings and model/version information; the publication review found no personal identifiers in the inspected data. It is included in the repository to reproduce the host tests, and excluded from the USB download. **Do not load it as a UEFI driver or flash it.** Host tests never execute its code.

`tests/guest.c` is a QEMU/OVMF integration harness. It publishes the same inert fixture as a synthetic Loaded Image, calls the shared engine and loads the **shipping driver** using real UEFI LoadImage/StartImage. The fixture's firmware entry point is never executed. A marker protocol tests late-execution refusal. The emulator has no laptop EC and cannot prove physical warning suppression.

To compile the emulator harness:

```bash
python3 -m ziglang cc -target x86_64-uefi-msvc -ffreestanding \
  -fno-stack-protector -fshort-wchar -mno-red-zone -fno-builtin \
  -mno-stack-arg-probe -Os -nostdlib -Wl,-e,EfiMain \
  tests/guest.c -o GuestTests.efi
```

Use an isolated emulator FAT volume with the bundled Shell at `EFI/BOOT/BOOTX64.EFI`, the harness, the shipping Driver.efi, and the fixture renamed `fixture.bin`. Run `GuestTests.efi` there; it shuts down the VM when complete. **Do not run the harness on the Lenovo.** It deliberately publishes synthetic firmware handles and changes only its allocated test pages. See `../docs/VALIDATION.md` for the actual test scope and skipped native RO-protocol test.

The code assumes the UEFI x64 pre-OS identity-mapped environment. It checks map ranges and current page-table mappings; it is not an OS kernel or runtime driver. Unsupported permissions, absent identity information, changed firmware text and late automatic execution cause refusal.
