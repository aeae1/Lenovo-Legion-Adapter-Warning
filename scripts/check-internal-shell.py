"""Exercise deployment scripts in disposable QEMU/OVMF disks, never host disks.

Requires qemu-system-x86_64, mtools, dosfstools, and OVMF.
V4_QEMU_PREFIX optionally selects an extracted package root instead of /usr.
"""
from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
work = root / 'build' / 'internal-shell-check'
work.mkdir(parents=True, exist_ok=True)
prefix = Path(os.environ.get('V4_QEMU_PREFIX', '/'))
env = dict(os.environ)
if prefix != Path('/'):
    env['LD_LIBRARY_PATH'] = str(prefix / 'usr/lib/x86_64-linux-gnu')
    env['QEMU_MODULE_DIR'] = str(prefix / 'usr/lib/x86_64-linux-gnu/qemu')


def run(args, **kwargs):
    return subprocess.run([str(a) for a in args], env=env, check=True, **kwargs)


def mtool(name, disk, *args):
    return run([prefix / 'usr/bin' / name, '-i', work / (disk + '.img'), *args],
               stdout=subprocess.PIPE, stderr=subprocess.PIPE)


startup = r'''@echo -off
connect -r
map -r
if exist fs0:\stage-internal.nsh then
  set -v src fs0:
  set -v dst fs1:
else
  set -v src fs1:
  set -v dst fs0:
endif
%src%
cd \
echo BEGIN_INTERNAL_DEPLOYMENT_CHECKS
comp mismatch-a.txt mismatch-b.txt
if %lasterror% == 0 then
  echo FAIL_COMPARE_DID_NOT_SIGNAL_DIFFERENCE
endif
mv LenovoWarning-v4-Driver.efi source-disabled.efi
stage-internal.nsh %dst% COPY
mv source-disabled.efi LenovoWarning-v4-Driver.efi
bcfg driver add 0 LenovoWarning-v4-Driver.efi "UNRELATED_VM_ENTRY"
bcfg driver add 0 LenovoWarning-v4-Driver.efi "Lenovo USB-C Warning Patch v4 TEST"
stage-internal.nsh
stage-internal.nsh %src% COPY
stage-internal.nsh %dst%
if exist %dst%\EFI\LenovoAdapterWarningV4 then
  echo FAIL_CHECK_ONLY_WROTE_FILES
endif
stage-internal.nsh %dst% COPY
stage-internal.nsh %dst% COPY
bcfg driver rm 0
bcfg driver add 0 %dst%\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi "Lenovo USB-C Warning Patch v4 INTERNAL"
bcfg driver dump -v
disable-internal.nsh %src% DISABLE
disable-internal.nsh %dst%
if not exist %dst%\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi then
  echo FAIL_CHECK_ONLY_DISABLED
endif
disable-internal.nsh %dst% DISABLE
disable-internal.nsh %dst% DISABLE
bcfg driver rm 0
echo FINAL_ENTRY_LIST
bcfg driver dump -v
echo END_INTERNAL_DEPLOYMENT_CHECKS
reset -s
'''

for disk in ('usb', 'internal'):
    with (work / (disk + '.img')).open('wb') as f:
        f.truncate(64 * 1024 * 1024)
    run([prefix / 'usr/sbin/mkfs.fat', '-F', '32', work / (disk + '.img')],
        stdout=subprocess.DEVNULL)
    mtool('mmd', disk, '::/EFI')
mtool('mmd', 'usb', '::/EFI/BOOT')
mtool('mcopy', 'usb', root / '03_AUTOMATIC_DRIVER_TEST/EFI/BOOT/BOOTX64.EFI',
      '::/EFI/BOOT/BOOTX64.EFI')
mtool('mcopy', 'usb', root / 'source/LenovoWarning-v4-Driver.efi', '::/')
for script in sorted((root / 'deployment/internal-1').glob('*.nsh')):
    mtool('mcopy', 'usb', script, '::/')
(work / 'startup.nsh').write_bytes(startup.replace('\n', '\r\n').encode('ascii'))
mtool('mcopy', 'usb', work / 'startup.nsh', '::/')
for name, content in [('mismatch-a.txt', 'A'), ('mismatch-b.txt', 'B')]:
    (work / name).write_text(content)
    mtool('mcopy', 'usb', work / name, '::/')
mtool('mmd', 'internal', '::/EFI/Microsoft', '::/EFI/Microsoft/Boot')
windows_marker = b'INERT WINDOWS PATH MARKER - NOT EXECUTABLE\r\n'
(work / 'bootmgfw.efi').write_bytes(windows_marker)
mtool('mcopy', 'internal', work / 'bootmgfw.efi', '::/EFI/Microsoft/Boot/bootmgfw.efi')
firmware = prefix / 'usr/share/OVMF'
shutil.copyfile(firmware / 'OVMF_VARS_4M.fd', work / 'vars.fd')
args = [prefix / 'usr/bin/qemu-system-x86_64', '-L', prefix / 'usr/share/qemu',
        '-machine', 'q35', '-m', '512M', '-vga', 'none', '-display', 'none',
        '-serial', 'stdio', '-monitor', 'none', '-net', 'none', '-no-reboot',
        '-drive', f'if=pflash,format=raw,readonly=on,file={firmware}/OVMF_CODE_4M.fd',
        '-drive', f'if=pflash,format=raw,file={work}/vars.fd',
        '-device', 'qemu-xhci',
        '-drive', f'if=none,id=usb,format=raw,file={work}/usb.img',
        '-device', 'usb-storage,drive=usb,bootindex=1',
        '-drive', f'if=none,id=int,format=raw,file={work}/internal.img',
        '-device', 'virtio-blk-pci,drive=int']
with (work / 'transcript.txt').open('wb') as f:
    run(args, stdout=f, stderr=subprocess.STDOUT, timeout=120)
transcript = (work / 'transcript.txt').read_text(errors='replace')
transcript = re.sub(r'\x1b\[[0-9;?]*[A-Za-z]', '', transcript)
for expected in ('BEGIN_INTERNAL_DEPLOYMENT_CHECKS', 'STOP_SOURCE:', 'STOP_TARGET:',
                 'CHECK_ONLY:', 'STAGED_OK:', 'STOP_EXISTS:', 'DISABLED_OK:',
                 'STOP_MISSING:', 'END_INTERNAL_DEPLOYMENT_CHECKS'):
    assert expected in transcript, f'Missing result: {expected}; see {work}/transcript.txt'
assert 'FAIL_' not in transcript, 'Shell check failed; inspect transcript'
assert 'Script Error' not in transcript, 'Shell parser error; inspect transcript'
assert 'Unknown flag' not in transcript, 'Shell option parsing error; inspect transcript'
assert 'bcfg: Cannot' not in transcript, 'Driver-entry operation failed; inspect transcript'
final = transcript.split('FINAL_ENTRY_LIST', 1)[1].split('END_INTERNAL_DEPLOYMENT_CHECKS', 1)[0]
assert 'UNRELATED_VM_ENTRY' in final and 'Lenovo USB-C Warning Patch' not in final
assert 'bcfg: Add' in transcript and 'INTERNAL' in transcript
for name in ('bootmgfw.efi', 'copied-driver.disabled'):
    (work / name).unlink(missing_ok=True)
mtool('mcopy', 'internal', '::/EFI/Microsoft/Boot/bootmgfw.efi', work / 'bootmgfw.efi')
assert (work / 'bootmgfw.efi').read_bytes() == windows_marker, 'Windows marker changed'
mtool('mcopy', 'internal', '::/EFI/LenovoAdapterWarningV4/LenovoWarning-v4-Driver.disabled',
      work / 'copied-driver.disabled')
assert hashlib.sha256((work / 'copied-driver.disabled').read_bytes()).hexdigest() == \
    '665d32265ce91946e58f9910f366db29682970d68ba4bcba7b749104c4382898'
listing = mtool('mdir', 'internal', '::/EFI/LenovoAdapterWarningV4/').stdout.decode()
assert 'LenovoWarning-v4-Driver.efi' not in listing, 'Active helper survived disable'
print('PASS isolated Shell paths, check-only, copy/readback, existing-folder refusal,')
print('     disable, missing-file refusal, driver-entry migration/removal, unrelated entry')
print('     preservation, helper SHA-256, and unchanged inert Windows path marker.')
print('LIMIT: this does not test Lenovo firmware, Windows, internal boot timing, or recovery.')
