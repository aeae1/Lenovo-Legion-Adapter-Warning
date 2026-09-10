# Lenovo Legion Adapter Warning

**Experimental UEFI research · GKCN65WW only · First warning-free boot reported; comparison pending**

A narrowly targeted attempt to suppress the startup adapter warning on the **Lenovo Legion 5 Pro 16ACH6H / 82JQ**. No simple warning-off NVRAM setting has been established.

V4 provides a diagnostic and two helpers that attempt a one-byte change to the loaded warning callback **in RAM for the current boot**. It does not flash the BIOS, approve a charger, increase USB-C power, or change charging limits. The helpers are unsigned.

## Start here

Download the [V4 public USB test prerelease](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/releases/tag/v4-public-1), then follow the [beginner walkthrough](docs/BEGINNER-WALKTHROUGH.md). It explains what happens, how to prepare the USB, Secure Boot, risk, and removal. The [short Phase 1 guide](START-HERE.md) is also available.

| Phase | What it establishes |
| --- | --- |
| **1 — Diagnostic** | Whether the loaded firmware matches and what its current permissions are. Writes a USB log; does not patch firmware or edit NVRAM. |
| **2 — Manual RAM test** | Whether the one-byte change and any required permission restoration work. Requires review of Phase 1. |
| **3 — Automatic USB test** | Whether DriverOrder starts the helper early enough, the warning disappears, and normal USB behavior remains intact. Requires a successful reviewed manual test. |

Use the normal Lenovo charger for the initial stages, or a sufficiently charged battery alone for the manual mechanics test; see [current test guidance](docs/STATUS.md). Review results before progressing. **Phase 1 does not establish automatic-driver timing, and a successful build does not establish a working laptop fix.** Review logs and photos for personal information before sharing them.

The public download contains the **same V4 EFI helper bytes** as the original test package. Its filename and ZIP checksum differ because firmware test data and personal context were removed. No replacement of an existing Phase 1 USB is necessary. See [public packaging](docs/PUBLIC-PACKAGE.md).

## Research and progress

| Record | Contents |
| --- | --- |
| [Current status](docs/STATUS.md) | Evidence, open questions, and physical-test status |
| [Tracking issues](planning/README.md) | Diagnostic, manual, automatic, Secure Boot, and deployment work |
| [Firmware reassessment](docs/FIRMWARE-REASSESSMENT.md) | Warning path, NVRAM candidates, and static boot ordering |
| [Comparison with V3](docs/COMPARISON-WITH-V3.md) | Earlier behavior and V4 corrections |
| [Older CPU protocol](docs/research/REVIEW-AND-CPU-PROTOCOL.md) | A possible permission-control route; not implemented or proven |
| [Validation](docs/VALIDATION.md) | Historical host/emulator results and their limits |
| [Hardware reports](hardware-results/README.md) | A template for recording actual observations |

## Build and check

On x64 Linux with Python 3 and GCC:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements-build.txt
bash scripts/check.sh
```

Public checks verify the package, rebuild all three helpers, require byte-identical V4 hashes, and run **33 host fault-injection cases** with address and undefined-behavior sanitizers. A reviewed 23,360-byte warning module is included as inert test data; its code is never executed by the tests. It is excluded from the USB download. See [source/README.md](source/README.md). Historical emulator results remain documented separately.

No complete BIOS image, personal hardware log, or private development history is published. The single reviewed firmware test module, compatibility signatures, and research offsets identify the supported firmware. The [publication review](docs/PUBLICATION-REVIEW.md) records the scope. See [contribution guidance](CONTRIBUTING.md) and [third-party notices](THIRD-PARTY.md).
