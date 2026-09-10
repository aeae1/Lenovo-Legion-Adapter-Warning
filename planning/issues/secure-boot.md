# Sign the V4 helper and establish Lenovo Secure Boot trust

**September 10, 2026: signing research in progress.** The owner has reported successful internal startup without the helper USB. The working installation uses the original unsigned V4 helper with Secure Boot disabled.

Offline signing tests pass: the intended certificate verifies, a wrong certificate and modified code are rejected, and the original executable payload is preserved. A disposable OVMF Secure Boot VM accepts the trusted signed helper and rejects unsigned, untrusted, and altered copies. These are development tests, not Lenovo enrollment or Secure Boot-enabled warning-suppression results.

The next laptop observation is the read-only Windows inventory in [the signing guide](../../signing/README.md#next-step-on-the-laptop-read-only-inventory). No permanent signing key, enrollment bundle, replacement installed driver, or key-reset instruction is supplied. Keep factory trust, revocations, and recovery information intact and private.

Remaining work: establish an accessible, supported Lenovo trust-enrollment route; review preservation and recovery; then validate early loading and warning suppression with Secure Boot enabled on the physical laptop. The supplied menu photographs do not establish an enrollment control. See [Secure Boot options](../../docs/SECURE-BOOT-OPTIONS.md).

Completion: a specific validated trust arrangement on this Lenovo, or a documented reason the requirement cannot be met. A signature alone does not complete this issue.
