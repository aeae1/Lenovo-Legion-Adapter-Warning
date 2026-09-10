# Lenovo Legion USB-C adapter warning: firmware reassessment

**Date:** September 10, 2026  
**Target:** Lenovo Legion 5 Pro 16ACH6H / 82JQ, GKCN65WW  
**Method:** Static extraction, x86-64 disassembly, and UEFI HII/IFR decoding of the analyzed firmware. No laptop execution or firmware modification.

## Assessment

**Suppressing this warning appears technically feasible without flashing the BIOS. I did not identify a simple NVRAM setting that disables it.** The most promising implementation remains a narrowly targeted EFI driver that suppresses the warning callback in RAM on each boot.

The new, useful finding is specific to this BIOS: its boot manager has an explicit DriverOrder load/start loop before the platform path that triggers the warning. This removes much of the previous uncertainty about whether an external driver could run early enough. It does not replace a real test of driver loading, target visibility, memory permissions, and cold-boot behavior on this laptop.

| Question | Finding | Confidence / limit |
|---|---|---|
| Dedicated setting to disable the warning? | None identified in the warning logic or decoded setup forms. | High confidence for the examined warning path; cannot prove no indirect behavior exists anywhere in proprietary firmware or the EC. |
| Can the display-and-Esc callback be suppressed? | It is a distinct, small callback with an identifiable entry point. | Strong static evidence; no hardware test. |
| Can an external EFI driver run before it? | DriverOrder processing precedes the relevant warning notification on the traced BDS path. | Strong static evidence; actual loading still needs validation. |
| Is this an easy BIOS-menu change? | No. | No appropriate switch found. |
| Could the finished solution be automatic? | Yes, if the driver approach passes testing and can be loaded under the chosen Secure Boot configuration. | Requires development and initial setup. |
| Does this require changing actual charging behavior? | The proposed callback suppression does not require changes to charger detection, EC power limits, or charging policy. | That is the implementation scope, not a tested hardware claim. |

## Input and reproducibility

The GKCN65WW analysis input was read without modification. The image itself is not distributed here.

- Length: **18,830,456 bytes**
- SHA-256: `2b872ef768d682ae933ee7f44a4d2515935d0a49b956ff9bb4cf7fc7f6b791a6`
- Embedded BIOS identifier: **GKCN65WW**
- Main firmware volume examined: image offset `0x856BB0`, length `0x360000`.
- Extracted executable modules from that volume and its nested content: **318**.
- Tools: `uefi_firmware` for extraction, `pefile` and Capstone for PE inspection/disassembly, IFRExtractor-RS **1.6.1** for setup forms.

This file is firmware-image evidence, not a live readout of the laptop's current NVRAM or embedded-controller state. The results apply to the analyzed version; a different installed BIOS needs separate verification.

Module hashes:

| Module | SHA-256 |
|---|---|
| AdapterUnsupportWarningPage | `6ea392a3d33a65e326a58e88025aec186daffc71ea7811edd5fce2ca68c00e9f` |
| BdsDxe | `ce4b3f57b0475b0d4bf90ee3d2fb19229cc368daf69bdea28b78c0554da29cc1` |
| SetupUtility | `5c17105aebbe0e004efad7f90c6e8a1884dff8bb4b897864a6a6546bf55c6d33` |

**All module addresses below are relative virtual addresses (RVAs) within the extracted PE module, not offsets in the complete BIOS.fd file. They are documentation, not flashing instructions.**

## What actually decides to show the warning

The responsible firmware file is:

- Name: `AdapterUnsupportWarningPage`
- FFS GUID: `0221D385-497A-4BBF-924E-7BDFC4C1EAD8`
- Extracted PE length: **23,360 bytes**
- PE entry point: `0x450`
- Adapter-specific initialization: `0xE90`
- Warning callback: `0x1030`

The entry wrapper calls library initialization and then the adapter-specific initialization. The adapter-specific routine accesses the EC through I/O ports `0x66` and `0x62`, issues command `0x80`, and reads EC address `0xA3`. It tests the returned byte with mask **0x30**. If both masked bits are zero, it returns without registering the two adapter-related notification callbacks.

The relevant conditional is:

```text
0xED9   mov al, 0xA3       ; EC address requested
...
0xEF1   in  al, dx         ; read from data port 0x62
0xEF2   test al, 0x30
0xEF4   je  0xFB6          ; skip registration if neither bit is set
```

When either masked bit is set, the routine registers:

1. A graphics-output-protocol notification with callback `0xFC0`.
2. A notification for GUID `3725E246-7CAB-4241-95A3-A560BDF44A6C`, with callback `0x1030`.

The second callback initializes/connects USB controllers, obtains the graphics-output protocol on the console handle, prints the adapter warning, and repeatedly reads keys until the scan code equals **0x17 (Esc)**. No warning-disable variable is read in that display-and-wait path. In particular, it is not waiting on the normal boot-menu timeout.

Simplified behavior, omitting error handling and library initialization:

```c
if ((ReadEcByte(0xA3) & 0x30) != 0) {
    RegisterGraphicsNotification(WriteIllegalAdapterStatus);
    RegisterPlatformNotification(DisplayWarningAndWaitForEsc);
}
```

I have not established the individual Lenovo meanings of EC bits 4 and 5. The code proves that their combined mask controls registration; it does not prove that one bit specifically means USB-C or a particular charger wattage.

## Why IllegalAdapter is not the switch we wanted

The driver contains a persistent-looking variable with exactly the tempting name **IllegalAdapter**:

- Variable name at module RVA `0x4E90`
- Namespace GUID: `6ACCE65D-DA35-4B39-B64B-5ED927A7DC7E`
- Callback: `0xFC0`

However, this callback first calls `SetVariable` with a one-byte value of **1**, size **1**, and attributes **7**. It then reads the variable back and returns. The readback value is not used to decide whether to install the warning callback or print the warning.

Therefore, setting `IllegalAdapter` to zero or deleting it is not supported as a suppression method by the recovered code. The callback writes it back to one, and the separate warning callback does not consult it.

Within the 318 extracted PE modules, the literal UTF-16 name `IllegalAdapter` occurred only in this module. That narrows the obvious uses; it does not exclude dynamically constructed names or EC behavior outside those modules.

The other obvious variable name in this module, `EFIDebug`, belongs to common initialization/debug support. There was no `SystemConfig` or `Setup` name in the warning module and no configuration-variable check in its adapter-specific gate or display callback.

## Hidden setup settings examined

The main SetupUtility yielded **nine form packages**. Because it embeds more than one English string package, IFRExtractor generated alternative pairings; the meaningful pairings were form packages **0–4 with string package 0**, and form packages **5–8 with string package 2**. Using the wrong pairing produces plausible-looking but incorrect setting labels.

Those meaningful packages cover Configuration, Power, Advanced, Home, Information, Main, Boot, Security, and Exit. Additional IFR content was decoded from AmdPbsSetupDxe, CbsSetupDxeRN, AodSetupDxe, and L05DxeServiceBody. SetupMenuService did not contain extractable IFR data.

| Candidate found | Stored location / evidence | Interpretation |
|---|---|---|
| Quiet Boot | SystemConfig `0x6E`; Boot form at `0x651E7` | Help text describes text-versus-graphical boot presentation. The warning callback has its own direct console output and does not test this setting. |
| Fast Boot | SystemConfig `0x231`; Boot form at `0x653A8` | Changes firmware initialization behavior. Not an identified warning-disable switch; could affect external-device availability during testing. |
| UEFI OS Fast Boot | SystemConfig `0x65`; Boot form at `0x654A2` | Help text concerns skipping keyboard initialization and firmware menu-key checks. It is not presented as suppression of adapter errors. |
| Instant boot | SystemConfig `0x2B3`; Configuration form at `0x346FA` | Controls automatic power-on when AC/USB-C PD is connected. Does not mean skipping this warning. |
| Charger mode (NVDC vs. BYPASS) | AMD_PBS_SETUP `0x1A`; form at `0xDDA0` | A charger-mode control, not a warning control. Its board-specific effect was not established here; it should not be treated as a cosmetic-message switch. |
| IllegalAdapter | Separate one-byte variable, described above | Output/status written by the warning driver, not a disable preference. |

The SystemConfig namespace is `A04A27F4-DF00-4D42-B552-39511302113D`; its main form-declared size is `0x2BC`. These offsets are recorded to distinguish the candidates, **not to recommend writing them**. No candidate above was established as a safe, effective way to suppress the warning.

The reason to decode IFR is that hidden settings can remain in the forms even when the setup interface hides them. [IFRExtractor-RS explains this relationship between forms and NVRAM settings.](https://github.com/LongSoft/IFRExtractor-RS)

## New finding: the EFI driver can plausibly get there in time

The strongest new evidence comes from **BdsDxe**, rather than the warning module alone.

Its internal load-option table at `0x2D640` associates type **0** with the strings **DriverOrder** and **Driver**. Its service table at `0x2D750` contains:

- Offset `0x48`: function `0x9140`, which requests the type-0 option list through `0x842C`.
- Offset `0x88`: function `0xA0BC`, which dispatches option execution.

In the boot-manager main routine:

1. At **0x30C4**, it calls the service-table method at offset `0x48` to obtain the driver list.
2. At **0x30EF**, it executes the entries through method offset `0x88`.
3. Driver execution follows `0xA0BC` into `0x8EEC`, with `LoadImage` at **0x8F52** and `StartImage` at **0x8FEE**. The path checks for a driver-compatible image code type.
4. Later, at **0x31A4**, the main routine calls the platform routine at **0x27C38**.
5. On its normal path, that platform routine calls **0x279BC** at **0x28045**.
6. At **0x27A02**, routine `0x279BC` installs the protocol GUID **3725E246-7CAB-4241-95A3-A560BDF44A6C**—the same notification GUID registered for the adapter-warning callback.

That is a concrete ordering relationship: **external driver loading precedes the identified platform warning trigger** on the traced path. It is stronger evidence than simply assuming all UEFI implementations behave the same.

There is another DriverOrder reference at `0x27E49`. That call is involved in load-option registration, not the decisive driver execution loop. It must not be mistaken for proof by itself.

For general context, UEFI specifies that DriverOrder entries are processed before BootOrder entries and that a Driver entry must identify an EFI driver, not an ordinary EFI application. [UEFI Specification, Boot Manager, sections 3.1.1–3.1.3.](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html)

A normal USB boot application can still run too late. The relevant proposal is an actual boot-service driver registered in **DriverOrder**, not merely putting an EFI file first in BootOrder.

## Practical implementation and its limits

The existing concept remains reasonable: locate the already loaded warning module, verify its identity and bytes, and make the event callback return immediately in memory. It should leave the EC-based power policy and the IllegalAdapter status callback alone.

The warning callback begins at RVA `0x1030` with:

```text
48 83 EC 38 E8 07 27 00 00 48 8B 0D 20 42 00 00
```

That 16-byte sequence occurred **once** among the 318 extracted PE modules. It is useful as an additional exact-version check, not as a substitute for identifying the correct loaded module and checking address bounds.

Because the target is an event-notification callback, an immediate return is a plausible minimal suppression. The earlier single-byte-in-RAM concept is consistent with this code. **No patched image or executable was produced or tested in this reassessment.**

Remaining practical questions:

- Does the laptop load the removable DriverOrder entry successfully on a cold boot?
- Is the target loaded image discoverable and does its signature match at that point?
- Is its code page writable, or is an appropriate supported memory-attribute change required?
- Does the warning stay absent without changing charging or normal startup behavior?
- Does removing the helper restore the original behavior?

An unsigned custom EFI driver needs a loading policy that allows it. Testing would normally mean disabling Secure Boot. Keeping Secure Boot enabled would require a properly trusted driver/signing arrangement; this analysis does not establish which enrollment route this Lenovo supports. Temporary disablement for a test does not mean an unsigned final helper will keep working after Secure Boot is re-enabled. UEFI's boot-manager specification explicitly notes that Secure Boot affects loading behavior. [UEFI Boot Manager specification.](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html)

The initial deployment should remain a removable-USB proof of concept with clear diagnostics and a removable DriverOrder entry. Only after it passes would an internal EFI-system-partition installation make sense. Installing the helper that way changes boot configuration and adds an EFI file; it does not require flashing modified motherboard firmware.

A RAM patch is not the same thing as risk-free software: a bad address or unsupported memory write can hang that boot. Its advantage here is avoiding a persistent BIOS rewrite and making rollback a matter of removing the helper/entry, if the implementation is constructed correctly.

## Recommendation

Do not spend time guessing NVRAM values for this warning. The direct firmware path does not support that approach.

The best next implementation task is a GKCN65WW-specific EFI boot-service driver with strict identity checks and a removable test installation. **Feasible-looking and potentially unobtrusive once installed; moderately involved to implement and validate; not yet a demonstrated fix.** Keeping Secure Boot enabled is an additional constraint, not a detail to promise away.
