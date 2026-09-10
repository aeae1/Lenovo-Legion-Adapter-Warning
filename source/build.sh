#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
# Reproducible compiler: python3 -m pip install ziglang==0.13.0
# Bundled Zig/Clang/LLD builds PE32+ UEFI binaries without an OS runtime.
for spec in '0 Diagnostic efi_application' '1 Manual efi_application' '2 Driver efi_boot_service_driver'; do
  read -r mode name subsystem <<< "$spec"
  python3 -m ziglang cc -target x86_64-uefi-msvc -ffreestanding -fno-stack-protector \
    -fshort-wchar -mno-red-zone -fno-builtin -mno-stack-arg-probe -Os \
    -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable -DMODE="$mode" \
    -nostdlib -Wl,-e,EfiMain -Wl,--subsystem,"$subsystem" \
    main.c -o "LenovoWarning-v4-$name.efi"
done
python3 normalize_pe.py LenovoWarning-v4-{Diagnostic,Manual,Driver}.efi
