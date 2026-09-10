"""Verify V4 binary identity, public archive selection, and excluded payloads."""
from pathlib import Path
import hashlib
import json
import subprocess
import sys
import zipfile

import pefile

root = Path(__file__).resolve().parents[1]
rebuild = Path(sys.argv[1]) if len(sys.argv) == 2 else root / 'source'
records = json.loads((root / 'docs/BINARY-VALIDATION.json').read_text())
expected_zip_sha = 'e546150399e4cb7f679201a244936019edaec580e16d4d19ccfc5e3e0f4c77a3'
shell_sha = 'e6c8c8b6ecf594927724894a394f12c1cb110faa393d6c0e01c1784f9df7cd7d'
allowed_efi = {v['sha256'] for v in records.values()} | {shell_sha}
allowed_images = {
    'docs/images/terminal-warning-banner.png': 'f7294410bee047cf9b998795bd3df17ad8ccebfa5f9fd8c3557e460d47a53547',
}


def require(ok, message):
    if not ok:
        raise SystemExit(message)


for name, record in records.items():
    for p in [root / 'source' / name, rebuild / name]:
        data = p.read_bytes()
        require(len(data) == record['bytes'], f'Size mismatch: {name}')
        require(hashlib.sha256(data).hexdigest() == record['sha256'], f'V4 hash mismatch: {name}')
        pe = pefile.PE(data=data)
        require(pe.FILE_HEADER.Machine == 0x8664 and pe.OPTIONAL_HEADER.Magic == 0x20B, f'Wrong architecture: {name}')
        require(pe.OPTIONAL_HEADER.Subsystem == record['subsystem'], f'Wrong subsystem: {name}')
        require(pe.OPTIONAL_HEADER.DATA_DIRECTORY[1].Size == 0, f'Imports present: {name}')
        require(pe.OPTIONAL_HEADER.DATA_DIRECTORY[4].Size == 0, f'Certificate changed: {name}')
        require(pe.OPTIONAL_HEADER.DATA_DIRECTORY[5].Size > 0, f'No relocations: {name}')
    print(f'PASS byte-identical V4 helper: {name}')

archive_rel = 'release-assets/Lenovo-GKCN65WW-V4-Public-USB-Test.zip'
internal_archive_rel = 'release-assets/Lenovo-GKCN65WW-V4-Internal-Deployment-1.zip'
allowed_archives = {
    archive_rel: expected_zip_sha,
    internal_archive_rel: '18fdccac56f4e0d78152b51a6b4c6479f63c7619726b2388bd18e271f55992ce',
}
subprocess.run([sys.executable, str(root / 'scripts/package-internal.py'), '--check'], check=True)
archive = root / archive_rel
require(hashlib.sha256(archive.read_bytes()).hexdigest() == expected_zip_sha, 'Public ZIP hash mismatch')
required = {line.split('  ', 1)[1] for line in (root / 'SHA256SUMS.txt').read_text().splitlines()} | {'SHA256SUMS.txt'}
with zipfile.ZipFile(archive) as z:
    entries = [i for i in z.infolist() if not i.is_dir()]
    require(len(entries) == len(required), 'Unexpected public archive file count')
    # The immutable release has its own manifest; current web docs may evolve.
    prefix = 'Lenovo-GKCN65WW-V4-Public-USB-Test/'
    archived_sums = dict((line.split('  ', 1)[1], line.split('  ', 1)[0])
                         for line in z.read(prefix + 'SHA256SUMS.txt').decode().splitlines())
    require(set(archived_sums) == required - {'SHA256SUMS.txt'}, 'Archive manifest selection differs')
    seen = set()
    for item in entries:
        parts = Path(item.filename).parts
        require(parts[0] == 'Lenovo-GKCN65WW-V4-Public-USB-Test', 'Wrong public archive root')
        name = Path(*parts[1:]).as_posix()
        require(name in required and name not in seen, f'Unexpected/duplicate archive member: {name}')
        seen.add(name)
        if name != 'SHA256SUMS.txt':
            require(hashlib.sha256(z.read(item)).hexdigest() == archived_sums[name],
                    f'Archive member checksum differs: {name}')
    require(seen == required, 'Missing public archive member')

names = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=root).decode().split('\0')
for name in set(filter(None, names)):
    p = root / name
    data = p.read_bytes()
    if name == 'source/tests/GKCN65WW-warning-fixture.bin':
        require(hashlib.sha256(data).hexdigest() == '6ea392a3d33a65e326a58e88025aec186daffc71ea7811edd5fce2ca68c00e9f', 'Unexpected firmware fixture')
        continue
    require(p.suffix.lower() not in {'.fd', '.rom', '.bin', '.bundle', '.log', '.key', '.pem', '.pfx', '.p12', '.auth', '.esl'}, f'Excluded private data: {name}')
    require(not any(marker in data for marker in (
        b'-----BEGIN ' + b'PRIVATE KEY-----',
        b'-----BEGIN RSA ' + b'PRIVATE KEY-----',
        b'-----BEGIN EC ' + b'PRIVATE KEY-----',
        b'-----BEGIN ENCRYPTED ' + b'PRIVATE KEY-----',
        b'-----BEGIN OPENSSH ' + b'PRIVATE KEY-----',
    )), f'Private key material: {name}')
    if p.suffix.lower() == '.zip':
        require(name in allowed_archives, f'Unexpected archive: {name}')
        require(hashlib.sha256(data).hexdigest() == allowed_archives[name], f'Archive hash mismatch: {name}')
    elif name in allowed_images:
        require(hashlib.sha256(data).hexdigest() == allowed_images[name], f'Unreviewed image: {name}')
    elif p.suffix.lower() == '.efi' or data.startswith(b'MZ'):
        require(hashlib.sha256(data).hexdigest() in allowed_efi, f'Unexpected executable payload: {name}')
    else:
        try:
            data.decode('utf-8')
        except UnicodeDecodeError:
            raise SystemExit(f'Unexpected binary data: {name}')
print('PASS public ZIP selection, fixed hash, and allowed binary payloads')
