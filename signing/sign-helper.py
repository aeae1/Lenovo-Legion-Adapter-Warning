"""Sign the frozen V4 driver offline. Never installs or enrolls anything."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess

import pefile

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = '665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898'


def verify_payload(original, signed):
    """Only the PE checksum/security directory may change before the appended cert."""
    old, new = pefile.PE(data=original), pefile.PE(data=signed)
    security = new.OPTIONAL_HEADER.DATA_DIRECTORY[4]
    if not security.Size or security.VirtualAddress < len(original):
        raise ValueError('Certificate must be appended after the original image')
    if security.VirtualAddress + security.Size > len(signed):
        raise ValueError('Certificate directory exceeds file')
    a, b = bytearray(original), bytearray(signed[:len(original)])
    for off, size in [(old.OPTIONAL_HEADER.get_field_absolute_offset('CheckSum'), 4),
                      (old.OPTIONAL_HEADER.DATA_DIRECTORY[4].get_file_offset(), 8)]:
        a[off:off+size] = b'\0' * size
        b[off:off+size] = b'\0' * size
    if a != b:
        raise ValueError('Signing changed original executable content')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--key', type=Path, required=True, help='Existing private RSA PEM key')
    parser.add_argument('--cert', type=Path, required=True, help='Matching public X.509 PEM certificate')
    parser.add_argument('--out-dir', type=Path, required=True, help='New private output directory')
    args = parser.parse_args()
    for tool in ('sbsign', 'sbverify'):
        if not shutil.which(tool):
            parser.error(f'Missing {tool}; install sbsigntool')
    key, cert = args.key.resolve(strict=True), args.cert.resolve(strict=True)
    out = args.out_dir.resolve()
    if ROOT == out or ROOT in out.parents:
        if ROOT / 'build' not in out.parents:
            parser.error('Inside this repository, output must be under ignored build/')
    source = ROOT / 'source/LenovoWarning-v4-Driver.efi'
    original = source.read_bytes()
    if hashlib.sha256(original).hexdigest() != EXPECTED:
        parser.error('The input does not match the frozen V4 driver')
    out.mkdir(parents=True, exist_ok=False, mode=0o700)
    target = out / 'LenovoWarning-v4-Driver.signed.efi'
    try:
        subprocess.run(['sbsign', '--key', str(key), '--cert', str(cert),
                        '--output', str(target), str(source)], check=True)
        subprocess.run(['sbverify', '--cert', str(cert), str(target)], check=True)
        verify_payload(original, target.read_bytes())
        if source.read_bytes() != original:
            raise ValueError('Original input changed unexpectedly')
    except Exception:
        target.unlink(missing_ok=True)
        raise
    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    (out / 'SHA256SUMS.txt').write_text(f'{digest}  {target.name}\n')
    print('PASS: signature verified and executable content preserved.')
    print('OFFLINE OUTPUT ONLY: not installed, not enrolled, not Lenovo-validated.')
    print(f'Signed helper: {target}')


if __name__ == '__main__':
    main()
