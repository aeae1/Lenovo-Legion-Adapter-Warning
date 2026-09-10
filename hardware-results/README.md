# Physical laptop results

**Diagnostic, manual, and first automatic runs reviewed.** See the [diagnostic report](phase-1-diagnostic-reviewed.md), [manual report](phase-2-manual-reviewed.md), and [initial automatic report](phase-3-automatic-initial-reviewed.md). The automatic helper found the trigger absent and verified its RAM patch. A [subsequent warning-free normal boot](phase-3-warning-free-reported.md) was reported with the helper USB connected and no F12. The [with/without/with comparison](phase-3-usb-comparison-reported.md) then reproduced warning-free / warning / warning-free behavior. Fresh run logs and USB peripheral checks remain pending.

Use the GitHub hardware-result issue template or copy this record into a new dated Markdown file after observations are available:

```text
Date:
Model and BIOS version:
Phase (1 diagnostic / 2 manual / 3 automatic / removal comparison):
Package or binary SHA-256:
Charger and attached peripherals:
Secure Boot state:
How started (F12 / manual application / DriverOrder):
Earlier Lenovo test-driver entries present or removed:
Exact result code:
Trigger at entry and before patch, if logged:
Page/descriptor permissions and protocol status, if logged:
Warning displayed (yes / no / not tested):
USB keyboard/storage/other peripheral behavior:
Shutdown, restart and removal observations:
Log or photograph reference:
What this observation establishes:
What it does not establish:
```

Keep raw logs locally until reviewed. Recovery keys, serial numbers, and complete firmware dumps do not belong in a result report. If a log is unavailable, say so and record the screen text; do not reconstruct missing values from expected results.
