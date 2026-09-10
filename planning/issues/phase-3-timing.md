# Test DriverOrder timing, warning suppression, and USB behavior

Depends on a reviewed successful manual stage.

Use the existing USB-hosted driver and exact-entry installation/removal instructions. Record trigger state at driver entry and before patch, actual automatic result, normal-charger behavior, and the planned USB-C comparison from the walkthrough. Check USB keyboard/storage and other attached peripherals because the skipped callback also contains a controller-connection call.

Completion: actual automatic logs and observed warning behavior, plus a comparison after removing the identified driver entry. Verify removal using the current displayed option number and exact entry description. Do not remove unrelated entries. Marker absence alone does not establish successful suppression. Do not treat an F12 diagnostic as this timing test.
