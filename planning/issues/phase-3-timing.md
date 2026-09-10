# Test DriverOrder timing, warning suppression, and USB behavior

**Completed milestone — September 10, 2026.** Two reviewed automatic USB logs found the warning trigger absent at entry and before the patch, then recorded a verified one-byte RAM change without permission transitions.

The owner reported warning absent / present / absent with the helper USB inserted / removed / reinserted under the same USB-C setup. This comparison demonstrates suppression in the tested configuration. The later internal migration showed the USB TEST entry removed and one INTERNAL entry present. Normal internal startup, mouse operation, and thumb-drive operation were subsequently reported.

See [the hardware index](../../hardware-results/README.md) for separate log reviews, owner reports, and internal setup observations. The warning comparison used removal of the helper USB; do not describe it as a separate warning comparison after deleting the saved driver entry.

Dock behavior, broad USB-controller compatibility, permission transitions, long-term behavior, and complete physical internal recovery remain untested. Results apply only to the tested Legion 5 Pro 16ACH6H / 82JQ on GKCN65WW. Secure Boot trust work continues in [issue #5](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/5).
