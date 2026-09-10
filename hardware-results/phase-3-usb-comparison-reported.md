# Helper USB removal/reinsertion comparison — reported September 10, 2026

**Outcome: helper-dependent warning suppression demonstrated in the reported comparison.**

After the first warning-free normal boot, the user followed up with two shutdown/start observations:

| Sequence | Helper USB | Adapter warning |
| --- | --- | --- |
| Initial normal boot | Connected | Absent |
| After shutdown and removal | Disconnected | Present |
| After shutdown and reinsertion | Connected | Absent |

The instructions held the same USB-C charger/cable constant, changed only the helper USB, and called for normal starts without F12. The user explicitly reported the shutdown/start and USB/warning outcomes; no changed power setting, duplicate installation, or entry recreation was reported. This is observational evidence, not instrumented power-state measurement.

Together with the reviewed successful automatic log, the result supports the intended explanation: the USB-hosted V4 helper applies the RAM change early enough to suppress the warning in this setup, and the effect depends on the helper being available. This is not a claim of a permanent BIOS change or compatibility with other versions/machines.

The fresh driver logs from the warning/comparison runs and normal USB peripheral behavior have not yet been supplied. The previous automatic log is not substituted for those later records. Exact charger/cable model, a separately measured USB binary hash, and long-term behavior remain undocumented.

[Issue #3](https://github.com/aeae1/Lenovo-Legion-Adapter-Warning/issues/3) remains open for the peripheral check and final archival log. The helper still resides on USB. Internal deployment and Secure Boot trust remain separate tasks. No source, helper binary, or published release asset changed for this report.
