"""Build the explicit Version 1.0.0 downloads; never replace an existing archive."""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import io
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'build/release-v1.0.0'
CHECKSUMS = ROOT / 'docs/releases/SHA256SUMS-v1.0.0.txt'
WEB = 'https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/blob/v1.0.0/'

# Each payload is selected explicitly. No source tree, logs, keys, or BIOS data.
USB_FILES = (
    '01_DIAGNOSTIC_USB/EFI/BOOT/BOOTX64.EFI',
    '01_DIAGNOSTIC_USB/LenovoWarning-v4-Diagnostic.efi',
    '02_MANUAL_PATCH_USB/EFI/BOOT/BOOTX64.EFI',
    '02_MANUAL_PATCH_USB/LenovoWarning-v4-Manual.efi',
    '03_AUTOMATIC_DRIVER_TEST/EFI/BOOT/BOOTX64.EFI',
    '03_AUTOMATIC_DRIVER_TEST/LenovoWarning-v4-Driver.efi',
    '03_AUTOMATIC_DRIVER_TEST/REMOVE-DRIVER.txt',
    '03_AUTOMATIC_DRIVER_TEST/TIANOCORE-SHELL-LICENSE.txt',
    '03_AUTOMATIC_DRIVER_TEST/install-driver.nsh',
    '03_AUTOMATIC_DRIVER_TEST/status.nsh',
)
FULL = {name: name for name in USB_FILES}
FULL.update({
    'START-HERE.md': 'docs/releases/v1.0.0-start-here.md',
    'USB-WALKTHROUGH.md': 'docs/V1-USB-WALKTHROUGH.md',
    'SIGNING-WALKTHROUGH.md': 'signing/WALKTHROUGH.md',
    'THIRD-PARTY.md': 'THIRD-PARTY.md',
    'INTERNAL_SETUP/READ-ME-FIRST.md': 'deployment/internal-1/READ-ME-FIRST.md',
    'INTERNAL_SETUP/USB/stage-internal.nsh': 'deployment/internal-1/stage-internal.nsh',
    'INTERNAL_SETUP/USB/disable-internal.nsh': 'deployment/internal-1/disable-internal.nsh',
    'INTERNAL_SETUP/USB/LenovoWarning-v4-Driver.efi': 'source/LenovoWarning-v4-Driver.efi',
    'INTERNAL_SETUP/USB/EFI/BOOT/BOOTX64.EFI': '03_AUTOMATIC_DRIVER_TEST/EFI/BOOT/BOOTX64.EFI',
    'INTERNAL_SETUP/USB/TIANOCORE-SHELL-LICENSE.txt': '03_AUTOMATIC_DRIVER_TEST/TIANOCORE-SHELL-LICENSE.txt',
})
INVENTORY = {
    'START-HERE.txt': 'signing/INVENTORY-START-HERE.txt',
    'SIGNING-WALKTHROUGH.md': 'signing/WALKTHROUGH.md',
    'Read-SecureBootState.cmd': 'signing/Read-SecureBootState.cmd',
    'Read-SecureBootState.ps1': 'signing/Read-SecureBootState.ps1',
}
PACKAGES = {
    'Lenovo-Legion-Adapter-Warning-v1.0.0': FULL,
    'Lenovo-Secure-Boot-Inventory-v1.0.0': INVENTORY,
}
EFI_HASHES = {
    '5bdea94cf293f5f135f869ff34efbd7c2c921a2254d9246dd452d01c5aaf1713',
    '54afa1d2dda25dcf9d355345640cf0c5773363920bd44770d9d70f6233dfa019',
    '665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898',
    'e6c8c8b6ecf594927724894a394f12c1cb110faa393d6c0e01c1784f9df7cd7d',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def payload_for(selected):
    payload = {}
    for dest, source in selected.items():
        path = PurePosixPath(dest)
        if path.is_absolute() or '..' in path.parts:
            raise SystemExit('Unsafe archive path')
        data = (ROOT / source).read_bytes()
        if path.suffix.lower() == '.efi':
            if digest(data) not in EFI_HASHES:
                raise SystemExit(f'Unreviewed EFI payload: {source}')
        else:
            text = data.decode('utf-8')
            if re.search(r'-----BEGIN (?:[A-Z]+ )*PRIVATE KEY-----', text):
                raise SystemExit(f'Private key material: {source}')
        # The internal guide moves in the ZIP. Keep its repository links usable.
        if dest == 'INTERNAL_SETUP/READ-ME-FIRST.md':
            data = data.decode('utf-8').replace('](../../', '](' + WEB).encode('utf-8')
        payload[dest] = data
    payload['SHA256SUMS.txt'] = ''.join(
        f'{digest(data)}  {dest}\n' for dest, data in sorted(payload.items())
    ).encode('ascii')
    return payload


def archive_bytes(name, payload):
    out = io.BytesIO()
    with zipfile.ZipFile(out, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for dest, data in sorted(payload.items()):
            item = zipfile.ZipInfo(f'{name}/{dest}', (2026, 9, 10, 0, 0, 0))
            item.compress_type = zipfile.ZIP_DEFLATED
            item.create_system = 3
            item.external_attr = 0o100644 << 16
            z.writestr(item, data, compresslevel=9)
    return out.getvalue()


def verify_archive(path, name, payload):
    with zipfile.ZipFile(path) as z:
        expected = {f'{name}/{dest}': data for dest, data in payload.items()}
        if len(z.infolist()) != len(expected) or set(z.namelist()) != set(expected):
            raise SystemExit(f'Unexpected members: {path.name}')
        for dest, data in expected.items():
            if z.read(dest) != data:
                raise SystemExit(f'Wrong archive bytes: {dest}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Verify existing outputs only')
    args = parser.parse_args()
    rendered = []
    for name, selected in PACKAGES.items():
        payload = payload_for(selected)
        data = archive_bytes(name, payload)
        rendered.append((name, payload, data))
    manifest = ''.join(f'{digest(data)}  {name}.zip\n' for name, _, data in rendered)
    if not CHECKSUMS.exists() or CHECKSUMS.read_text() != manifest:
        raise SystemExit('Release inputs differ from the reviewed checksum record:\n' + manifest)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for name, payload, data in rendered:
        path = OUTPUT / (name + '.zip')
        if not path.exists() and not args.check:
            with path.open('xb') as f:
                f.write(data)
        if not path.exists() or path.read_bytes() != data:
            raise SystemExit(f'Missing/different output; refusing overwrite: {path.name}')
        verify_archive(path, name, payload)
        print(f'PASS {path.name}: {len(payload)} explicit files; SHA-256 {digest(data)}')
    sums = OUTPUT / 'SHA256SUMS-v1.0.0.txt'
    if not sums.exists() and not args.check:
        with sums.open('x') as f:
            f.write(manifest)
    if not sums.exists() or sums.read_text() != manifest:
        raise SystemExit('Missing/different output checksums')
    print('PASS Version 1.0.0 downloads and per-file manifests')


if __name__ == '__main__':
    main()
