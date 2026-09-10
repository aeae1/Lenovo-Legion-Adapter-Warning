"""Build or verify the separately versioned internal-deployment archive."""
from pathlib import Path
import hashlib
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
name = 'Lenovo-GKCN65WW-V4-Internal-Deployment-1'
archive = root / 'release-assets' / (name + '.zip')
selected = {
    'READ-ME-FIRST.md': 'deployment/internal-1/READ-ME-FIRST.md',
    'SECURE-BOOT-OPTIONS.md': 'docs/SECURE-BOOT-OPTIONS.md',
    'USB/stage-internal.nsh': 'deployment/internal-1/stage-internal.nsh',
    'USB/disable-internal.nsh': 'deployment/internal-1/disable-internal.nsh',
    'USB/LenovoWarning-v4-Driver.efi': 'source/LenovoWarning-v4-Driver.efi',
    'USB/EFI/BOOT/BOOTX64.EFI': '03_AUTOMATIC_DRIVER_TEST/EFI/BOOT/BOOTX64.EFI',
    'USB/TIANOCORE-SHELL-LICENSE.txt': '03_AUTOMATIC_DRIVER_TEST/TIANOCORE-SHELL-LICENSE.txt',
}
payload = {dest: (root / src).read_bytes() for dest, src in selected.items()}
payload['SHA256SUMS.txt'] = ''.join(
    f'{hashlib.sha256(data).hexdigest()}  {dest}\n'
    for dest, data in sorted(payload.items())
).encode('ascii')
if '--check' not in sys.argv:
    archive.parent.mkdir(exist_ok=True)
    if archive.exists():
        raise SystemExit('Archive already exists; use --check. Published versions are immutable.')
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for dest, data in sorted(payload.items()):
            item = zipfile.ZipInfo(f'{name}/{dest}', (2026, 9, 10, 0, 0, 0))
            item.compress_type = zipfile.ZIP_DEFLATED
            item.create_system = 3
            item.external_attr = 0o100644 << 16
            z.writestr(item, data, compresslevel=9)
    sha = hashlib.sha256(archive.read_bytes()).hexdigest()
    (root / 'release-assets/SHA256SUMS-v4-internal-1.txt').write_text(f'{sha}  {archive.name}\n')
with zipfile.ZipFile(archive) as z:
    expected = {f'{name}/{dest}': data for dest, data in payload.items()}
    assert len(z.infolist()) == len(expected), 'Unexpected archive members'
    assert set(z.namelist()) == set(expected), 'Missing or extra archive member'
    for path, data in expected.items():
        assert z.read(path) == data, f'Archive content differs: {path}'
record = (root / 'release-assets/SHA256SUMS-v4-internal-1.txt').read_text().strip()
assert record == f'{hashlib.sha256(archive.read_bytes()).hexdigest()}  {archive.name}'
print(f'PASS internal deployment archive: {len(payload)} explicit files, matching bytes and checksum')
