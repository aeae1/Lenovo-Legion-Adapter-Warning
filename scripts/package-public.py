"""Build only the explicit, firmware-free public USB package."""
from pathlib import Path
import hashlib
import zipfile

root = Path(__file__).resolve().parents[1]
# This filename was published. Never replace it with edited guide contents.
if (root / 'release-assets/Lenovo-GKCN65WW-V4-Public-USB-Test.zip').exists():
    raise SystemExit('Historical release already exists; use a new release name and checksums for changed contents.')
stages = ['01_DIAGNOSTIC_USB', '02_MANUAL_PATCH_USB', '03_AUTOMATIC_DRIVER_TEST']
paths = [p.relative_to(root).as_posix() for name in stages for p in (root / name).rglob('*') if p.is_file()]
paths += ['START-HERE.md', 'WALKTHROUGH.md', 'README-FIRST.txt', 'verify-package.py',
          'docs/BEGINNER-WALKTHROUGH.md', 'docs/PUBLIC-PACKAGE.md']
paths = sorted(paths)
manifest = ''.join(f'{hashlib.sha256((root / name).read_bytes()).hexdigest()}  {name}\n' for name in paths)
(root / 'SHA256SUMS.txt').write_text(manifest)
archive = root / 'release-assets/Lenovo-GKCN65WW-V4-Public-USB-Test.zip'
archive.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for name in sorted(paths + ['SHA256SUMS.txt']):
        info = zipfile.ZipInfo('Lenovo-GKCN65WW-V4-Public-USB-Test/' + name, (2026, 9, 10, 0, 0, 0))
        info.compress_type = zipfile.ZIP_DEFLATED
        info.external_attr = 0o100644 << 16
        z.writestr(info, (root / name).read_bytes(), compresslevel=9)
digest = hashlib.sha256(archive.read_bytes()).hexdigest()
(archive.parent / 'SHA256SUMS-v4-public.txt').write_text(f'{digest}  {archive.name}\n')
print(f'Created {archive.name}: {len(paths) + 1} files; SHA-256 {digest}')
