# Project status — September 10, 2026

**Version 1.0.0 is the latest regular release of the tested unsigned setup. Existing internal V4 installations use the same helper and need no reinstall. Secure Boot support remains in development.**

**Internal-drive startup without the helper USB is now reported working on the tested GKCN65WW laptop. Mouse and thumb-drive checks also passed according to the owner. Secure Boot signing research has started; the working installation remains unsigned with Secure Boot disabled.**

| Question | Evidence and current conclusion |
| --- | --- |
| Easy warning-off NVRAM preference? | None established. `IllegalAdapter` is status, not an identified disable preference. |
| Intended module recognized and writable? | Diagnostic, manual, and automatic logs matched the expected module. Target mapping was writable; no memory-permission change was needed. |
| Automatic driver early enough? | Both reviewed automatic USB logs found the trigger absent at entry and before patch, then recorded patched=1, verified=1, status=0, rollback=0. |
| Warning suppressed? | With/without/with helper-USB comparison reported absent/present/absent under the same USB-C setup. |
| USB-free normal startup? | Internal copy compared successfully; exactly one INTERNAL entry on an NVMe path was shown. Normal startup without the helper USB or F12 was reported. No full internal-run log supplied. |
| USB behavior? | Owner reported mouse and thumb drive working after internal startup. Dock and broader controller behavior remain untested. |
| Secure Boot enabled on Lenovo? | Not yet. Offline signature verification and disposable OVMF enforcement tests passed. Firmware trust enrollment and Lenovo behavior with Secure Boot on are unresolved. |
| Permission fallback needed? | Not for the observed writable-page runs. Older CPU protocol research is deferred; no fallback or CR0.WP bypass was added. |

## Physical evidence

The [hardware index](../hardware-results/README.md) links the diagnostic, manual, automatic, comparison, and internal-deployment records. The latest USB automatic log agrees with the first: marker absent twice, verified RAM patch, no permission changes. The internal screenshots show a successful byte comparison and migration from the USB TEST entry to an internal NVMe INTERNAL entry. The owner's subsequent reports establish normal USB-free startup and basic peripheral operation in that setup.

The exact charger/cable model, separately measured executable hash on the physical USB, dock behavior, long-term behavior, and physical internal disable/recovery procedure have not been documented. Do not generalize the observations to other firmware or machines. No further repeat of the completed with/without/with test is needed merely to restate it.

## Secure Boot project

The actual firmware menu shows Secure Boot disabled, Platform Mode User Mode, and Secure Boot Mode Standard. Reset to Setup Mode and Restore Factory Keys are visible; no file-approval or certificate-enrollment menu is visible in the supplied photos. Static enrollment strings are a research lead, not proof of an accessible menu.

New tooling signs an offline copy of the frozen driver, checks the signature, and verifies that the original executable payload was preserved. Disposable tests reject a wrong certificate and altered code. In an isolated OVMF machine with SecureBoot=1 and SetupMode=0, the trusted signed helper loaded and started without a Lenovo target; unsigned, untrusted, and altered copies were refused. No real firmware variables were written by these tests.

The [Windows inventory](../signing/README.md#next-step-on-the-laptop-read-only-inventory) only reads Secure Boot state and exports trust databases locally. It is the next proposed laptop observation. No permanent signing key, enrollment bundle, or replacement installed driver has been created. See [Secure Boot options](SECURE-BOOT-OPTIONS.md) and [signing validation](../signing/README.md#developer-validation).

## Validation and releases

Original V4 binaries/source and both published ZIPs remain unchanged. Public checks retain byte-identical rebuilds and 33 sanitized host cases. The original synthetic Lenovo-target integration, internal Shell-script VM checks, and new Secure Boot admission VM checks answer different questions; none is a substitute for laptop observations.

The original emulator permission-transition test was skipped because the relevant protocol was unavailable. Physical permission transitions remain untested and were unnecessary on the observed machine. Existing removal instructions are available; their complete physical recovery path has not been exercised.

[Issues #1, #2, #3, and #6](../planning/README.md) record completed diagnostic/manual/automatic/internal milestones. #5 tracks Secure Boot; #4 remains deferred research. Historical guides embedded in published ZIPs retain their original checkpoint text. Current web guides and release notes reflect the newer results.
