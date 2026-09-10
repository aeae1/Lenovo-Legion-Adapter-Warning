# Plan USB-free deployment and removal only after a successful USB experiment

Depends on a successful reviewed automatic USB test, warning/no-helper comparison, peripheral checks, and a decision about Secure Boot.

The current package intentionally supplies a USB test. Internal deployment is not implemented. Document a future location, exact driver-load option, file identity, removal method, and recovery if a file or entry becomes unavailable. Check for older entries and preserve unrelated boot configuration.

Completion: a reviewed concrete deployment/removal design and a separately versioned implementation if pursued. Do not copy the experimental helper onto the internal EFI partition merely because the repository checks passed.
