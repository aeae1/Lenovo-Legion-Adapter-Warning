# Working on this project

Read README.md and docs/STATUS.md. Use plain language and distinguish static analysis, host tests, emulation, and actual laptop observations.

- The owner authorized public publication with scrubbing. Preserve V4 helper source and EFI binaries byte for byte while hardware testing is pending. Documentation and public packaging can change; give a changed archive a distinct name and checksum.
- Never upload the original private ZIP, private git history, full BIOS images, unreviewed firmware modules, raw personal logs, recovery keys, serial numbers, or workstation paths. The single reviewed fixture at source/tests/GKCN65WW-warning-fixture.bin is explicitly allowed; verify its recorded hash.
- The public ZIP contains only explicitly selected USB files and guides. Regenerate its manifest if those files change, and use a new release version after publication. Do not replace published assets silently.
- Scope is the documented model and GKCN65WW fingerprint. Do not weaken identity, permission, timing, or restoration checks. Do not silently add firmware flashing, EC writes, speculative NVRAM edits, or CR0.WP bypasses.
- Phase 1 only diagnoses current state and writes a USB log. It does not establish DriverOrder timing. Shell DriverOrder installation/removal is a separate persistent change.
- The older CPU protocol is an untested research lead. Public CI runs the 33 host cases; never describe a synthetic target or binary rebuild as a physical success.
- Run scripts/check.sh for relevant infrastructure changes. Keep historical validation separate from new results. Read hardware reports before changing physical-test status.

User instructions take precedence over this guidance.
