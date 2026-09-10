"""Local signing mechanics with disposable keys; never enrolls or publishes keys."""
from pathlib import Path
import hashlib
import os
import subprocess
import tempfile
import sys

ROOT = Path(__file__).resolve().parents[1]
work = Path(tempfile.mkdtemp(prefix='signing-test-', dir=ROOT / 'build'))
os.chmod(work, 0o700)


def run(args, good=True):
    p = subprocess.run([str(x) for x in args], capture_output=True, text=True)
    if (p.returncode == 0) != good:
        raise RuntimeError(f'Unexpected return code {p.returncode}: {args[0]}\n{p.stdout}\n{p.stderr}')
    return p


try:
    for label in ('trusted', 'other'):
        run(['openssl', 'req', '-new', '-x509', '-newkey', 'rsa:2048', '-sha256',
             '-nodes', '-days', '2', '-subj', '/CN=DISPOSABLE V4 SIGNING TEST ONLY/',
             '-keyout', work / (label + '.key'), '-out', work / (label + '.pem')])
        os.chmod(work / (label + '.key'), 0o600)
    run([sys.executable, ROOT / 'signing/sign-helper.py', '--key', work / 'trusted.key',
         '--cert', work / 'trusted.pem', '--out-dir', work / 'signed'])
    signed = work / 'signed/LenovoWarning-v4-Driver.signed.efi'
    run(['sbverify', '--cert', work / 'trusted.pem', signed])
    run(['sbverify', '--cert', work / 'other.pem', signed], good=False)
    raw = bytearray(signed.read_bytes())
    import pefile
    pe = pefile.PE(data=raw)
    raw[pe.sections[0].PointerToRawData + 16] ^= 1
    tampered = work / 'tampered.efi'
    tampered.write_bytes(raw)
    run(['sbverify', '--cert', work / 'trusted.pem', tampered], good=False)
    run([sys.executable, ROOT / 'signing/sign-helper.py', '--key', work / 'trusted.key',
         '--cert', work / 'trusted.pem', '--out-dir', work / 'signed'], good=False)
    run([sys.executable, ROOT / 'signing/sign-helper.py', '--key', work / 'trusted.key',
         '--cert', work / 'other.pem', '--out-dir', work / 'mismatch'], good=False)
    assert not (work / 'mismatch/LenovoWarning-v4-Driver.signed.efi').exists()
    assert hashlib.sha256((ROOT / 'source/LenovoWarning-v4-Driver.efi').read_bytes()).hexdigest() == \
        '665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898'
    print('PASS signed helper verifies; wrong certificate and tampered code rejected;')
    print('     original payload preserved; existing output refused; mismatched-key output removed.')
finally:
    for key in work.glob('*.key'):
        key.unlink()
    print('Disposable private test keys deleted. No enrollment or laptop modification occurred.')
