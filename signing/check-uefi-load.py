"""Disposable Secure Boot VM: signed/unsigned driver admission, no host disks."""
from pathlib import Path
import hashlib
import os
import re
import subprocess
import sys
import tempfile
import pefile

root = Path(__file__).resolve().parents[1]
work = Path(tempfile.mkdtemp(prefix='secure-boot-vm-', dir=root / 'build'))
os.chmod(work, 0o700)
prefix = Path(os.environ.get('V4_QEMU_PREFIX', '/'))
env = dict(os.environ)
if prefix != Path('/'):
    env['LD_LIBRARY_PATH'] = str(prefix / 'usr/lib/x86_64-linux-gnu')
    env['QEMU_MODULE_DIR'] = str(prefix / 'usr/lib/x86_64-linux-gnu/qemu')


def run(args, **kw):
    return subprocess.run([str(x) for x in args], env=env, check=True,
                          stdout=kw.pop('stdout', subprocess.PIPE),
                          stderr=kw.pop('stderr', subprocess.PIPE), **kw)


def sign(source, target, label='trusted'):
    run(['sbsign', '--key', work / (label + '.key'), '--cert', work / (label + '.pem'),
         '--output', target, source])


try:
    original = root / 'source/LenovoWarning-v4-Driver.efi'
    assert hashlib.sha256(original.read_bytes()).hexdigest() == \
        '665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898'
    for label in ('trusted', 'other'):
        run(['openssl', 'req', '-new', '-x509', '-newkey', 'rsa:2048', '-sha256',
             '-nodes', '-days', '2', '-subj', '/CN=DISPOSABLE VM TEST ONLY/',
             '-keyout', work / (label + '.key'), '-out', work / (label + '.pem')])
        os.chmod(work / (label + '.key'), 0o600)
    run([sys.executable, '-m', 'ziglang', 'cc', '-target', 'x86_64-uefi-msvc',
         '-ffreestanding', '-fno-stack-protector', '-fshort-wchar', '-mno-red-zone',
         '-fno-builtin', '-mno-stack-arg-probe', '-Os', '-Wall', '-Wextra', '-Werror',
         '-Wno-unused-function', '-Wno-unused-variable', '-nostdlib', '-Wl,-e,EfiMain',
         '-Wl,--subsystem,efi_application', root / 'signing/vm-load-check.c', '-o', work / 'probe.efi'])
    sign(work / 'probe.efi', work / 'BOOTX64.EFI')
    for label in ('trusted', 'other'):
        sign(original, work / (label + '.efi'), label)
    raw = bytearray((work / 'trusted.efi').read_bytes())
    pe = pefile.PE(data=raw)
    raw[pe.sections[0].PointerToRawData + 16] ^= 1
    (work / 'tampered.efi').write_bytes(raw)
    firmware = prefix / 'usr/share/OVMF'
    # Edit ONLY this new disposable OVMF variables file, never physical NVRAM.
    run([sys.executable, '-m', 'virt.firmware.vars', '--input', firmware / 'OVMF_VARS_4M.fd',
         '--output', work / 'vars.fd', '--enroll-cert', work / 'trusted.pem',
         '--microsoft-db', 'none', '--microsoft-kek', 'none',
         '--add-db', '11111111-2222-3333-4444-555555555555', work / 'trusted.pem', '--sb'])
    disk = work / 'disk.img'
    with disk.open('wb') as f:
        f.truncate(64 * 1024 * 1024)
    run([prefix / 'usr/sbin/mkfs.fat', '-F', '32', disk])
    run([prefix / 'usr/bin/mmd', '-i', disk, '::/EFI', '::/EFI/BOOT'])
    files = [(work / 'BOOTX64.EFI', '::/EFI/BOOT/BOOTX64.EFI'), (original, '::/unsigned.efi')]
    files += [(work / (n + '.efi'), '::/' + n + '.efi') for n in ('trusted', 'other', 'tampered')]
    for src, dest in files:
        run([prefix / 'usr/bin/mcopy', '-i', disk, src, dest])
    args = [prefix / 'usr/bin/qemu-system-x86_64', '-L', prefix / 'usr/share/qemu',
            '-machine', 'q35,smm=on', '-m', '512M', '-vga', 'none', '-display', 'none',
            '-serial', 'stdio', '-monitor', 'none', '-net', 'none', '-no-reboot',
            '-global', 'driver=cfi.pflash01,property=secure,value=on',
            '-drive', f'if=pflash,format=raw,readonly=on,file={firmware}/OVMF_CODE_4M.secboot.fd',
            '-drive', f'if=pflash,format=raw,file={work}/vars.fd',
            '-device', 'qemu-xhci', '-drive', f'if=none,id=usb,format=raw,file={disk}',
            '-device', 'usb-storage,drive=usb,bootindex=1']
    with (work / 'transcript.txt').open('wb') as f:
        run(args, stdout=f, stderr=subprocess.STDOUT, timeout=90)
    text = re.sub(r'\x1b\[[0-9;?]*[A-Za-z]', '', (work / 'transcript.txt').read_text(errors='replace'))
    assert 'BEGIN_SECURE_BOOT_VM_LOAD_CHECK' in text and 'FAIL ' not in text, text
    assert 'SECURE_BOOT_VM_FAILURES=0x0000000000000000' in text, text
    assert 'PASS trusted signed helper accepted' in text, text
    print('\n'.join(line for line in text.splitlines() if line.startswith(('PASS ', 'SECURE_BOOT_VM_FAILURES'))))
    print('LIMIT: OVMF test keys and no Lenovo target. Not a laptop enrollment test.')
finally:
    for key in work.glob('*.key'):
        key.unlink()
    print('Disposable private VM keys deleted.')
