"""Optional: python3 verify-package.py, from any directory. Reads files only."""
from pathlib import Path
import hashlib

root = Path(__file__).resolve().parent
count = 0
for line in (root / 'SHA256SUMS.txt').read_text().splitlines():
    expected, name = line.split('  ', 1)
    path = (root / name).resolve()
    if not path.is_relative_to(root):
        raise SystemExit('Unsafe manifest path')
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        raise SystemExit('CHECKSUM MISMATCH: ' + name)
    count += 1
print(f'All {count} package files match their checksums.')
