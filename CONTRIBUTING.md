# Making and reviewing changes

Use a branch and explain the concrete problem, changed behavior, verification, and remaining uncertainty. Keep V4 helper source and binaries intact while hardware testing is pending. Future helpers need new version names and an explicit comparison.

Run `bash scripts/check.sh` with the pinned dependencies. Public checks rebuild and verify V4 and run the 33 host cases against the reviewed inert fixture. Do not weaken a gate or fetch firmware from an unofficial source just to make CI green.

Record actual hardware evidence in a tracking issue using the hardware-result template. Include phase, BIOS version, binary hash, charger, entry method, result, observed warning behavior, USB behavior, and removal outcome. Review logs/photos for personal information before sharing. Do not include recovery keys, serial numbers, raw dumps, or additional unreviewed firmware.

This is a public repository. The original private ZIP and private git history must never be pushed, even to a temporary branch. Public downloads use explicit file lists, contain the unchanged helper binaries, and omit the Lenovo test fixture. Keep published release assets immutable; use a new packaging revision for a changed archive.

No project-wide open-source license has been selected. Preserve all third-party notices and do not apply a blanket license to material from other projects.
