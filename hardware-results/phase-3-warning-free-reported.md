# First warning-free normal boot — reported September 10, 2026

The user reported that the adapter warning did not appear, the helper USB remained connected, and F12 was not used. This report followed the instructions to test with the USB-C charger/cable that normally produces the warning.

This is the first direct report of the desired visible behavior. The previously supplied automatic log separately establishes an absent trigger at both samples and a verified RAM patch. The fresh log from this subsequent warning-free run has not yet been supplied; the two observations must not be represented as one log record.

Exact charger/cable details, normal Windows and USB peripheral behavior, and the full-shutdown comparison with the helper absent/reinserted remain pending. Keep the same USB-C power setup for that comparison, remove only the helper USB between the first pair of boots, and reinsert it in the same port for the last boot. Expected behavior is the warning returning without the helper and disappearing with it. If the outcome differs, record it and inspect driver registration before repeating installation.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open. No new helper, BIOS image change, internal installation, or Secure Boot trust arrangement is claimed. Public notes omit personal data and raw device identifiers.
