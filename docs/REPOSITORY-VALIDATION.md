# Public repository validation

The public tree starts from the repository's original README commit. The private preparation commits and original private ZIP are excluded from all public branches and release assets. One reviewed warning-module fixture is included in source/tests; it is not in the USB download.

Local verification checks the public package manifest, exact V4 helper hashes and PE format, byte-identical rebuilds, allowed binary payloads, and archive contents. The existing 33 host cases run with the reviewed fixture in source/tests under AddressSanitizer and UndefinedBehaviorSanitizer. Historical results are recorded in `VALIDATION.md`.

Public packaging changes documents and file selection, not the V4 helper implementation. The public release uses the distinct tag `v4-public-1` and filename `Lenovo-GKCN65WW-V4-Public-USB-Test.zip`.

GitHub workflow outcomes are available in the repository's Actions tab. A build/check result is not a physical laptop result. Hardware validation remains pending until actual observations are recorded.
