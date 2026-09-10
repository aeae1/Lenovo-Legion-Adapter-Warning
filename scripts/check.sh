#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
python3 verify-package.py
mkdir -p build/rebuild
python3 - <<'CHECKPY'
from pathlib import Path
import shutil
shutil.copytree('source', 'build/rebuild', dirs_exist_ok=True)
CHECKPY
bash build/rebuild/build.sh
python3 scripts/verify-public.py build/rebuild
gcc -std=c11 -O1 -g -fshort-wchar -fsanitize=address,undefined \
  -fno-omit-frame-pointer source/tests/host.c -o build/host-tests
ASAN_OPTIONS=detect_leaks=0 build/host-tests source/tests/GKCN65WW-warning-fixture.bin | tee build/host-tests.log
python3 - <<'CHECKPY'
from pathlib import Path
if 'TOTAL 33 host fault-injection cases passed' not in Path('build/host-tests.log').read_text():
    raise SystemExit('Expected all 33 original host cases')
CHECKPY
