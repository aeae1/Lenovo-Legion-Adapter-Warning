# Physical laptop results

**The internal installation now starts normally without the helper USB; the owner also reported working mouse and thumb-drive operation.** These are single-machine observations with Secure Boot disabled.

| Record | What it establishes |
| --- | --- |
| [Phase 1 diagnostic](phase-1-diagnostic-reviewed.md) | Module identity and current writable mapping |
| [Phase 2 manual](phase-2-manual-reviewed.md) | Verified one-byte RAM change |
| [First automatic log](phase-3-automatic-initial-reviewed.md) | Early timing marker absence and verified automatic patch |
| [Normal USB startup](phase-3-warning-free-reported.md) | Warning-free normal startup without F12 reported |
| [USB comparison](phase-3-usb-comparison-reported.md) | Warning absent/present/absent with/without/with helper USB |
| [Internal deployment and peripherals](phase-4-internal-and-usb-reviewed.md) | Internal copy/entry reviewed; normal USB-free startup and mouse/storage reported |
| [Secure Boot menu](secure-boot-menu-reviewed.md) | Current disabled/User Mode/Standard state and visible reset actions |

The latest additional automatic USB log was reviewed after the comparison: marker absent at entry and before patch; patched=1, verified=1, status=0, rollback=0; no permission changes. A full internal-run log has not been supplied. Older reports preserve the evidence known at their checkpoint; [current status](../docs/STATUS.md) supersedes their pending-work notes.

For future reports, use the GitHub hardware-result template. Include model/BIOS, package hash if measured, power setup, Secure Boot state, how the helper started, exact result, warning behavior, peripherals, and what was not tested. Never reconstruct missing logs from expected values. Keep raw logs and photographs private until reviewed for serial numbers, partition identifiers, recovery keys, and other personal data.
