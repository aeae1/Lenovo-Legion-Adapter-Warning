# Troubleshooting without a support appointment

These guides are self-service. You do not need to submit logs or obtain the maintainer's approval. Individual installation or firmware-recovery support is not offered by this project.

## Check the result before moving on

Use the [current staged USB guide](V1-USB-WALKTHROUGH.md) and compare every listed field in the newest complete log record for the stage you ran. Diagnostic success does not prove automatic timing. Manual success does not prove warning suppression. Stop progression on mismatches, errors, failed restoration, or an unexplained missing log. Do not change the supported firmware fingerprint or bypass a refusal.

For an ambiguous internal EFI partition, keep the working USB arrangement. Compare the Windows boot entry's device path with the Shell filesystem map as described in the [internal guide](../deployment/internal-1/READ-ME-FIRST.md). The presence of bootmgfw.efi alone does not distinguish an active installation from an old copy. Never format a partition, delete another bootloader, or choose a filesystem number by guesswork.

## Ask an LLM or a technician if needed

Your friendly neighborhood LLM can help explain a result. Give it the repository link, exact laptop model and BIOS version, the guide section, the command you ran, and the relevant error or newest complete log record. For example:

> Help me interpret this Lenovo adapter-warning helper result using the project's current documentation. Explain which published success criteria pass or fail. Identify missing evidence before suggesting a next step; do not guess drive numbers or suggest bypassing firmware checks.

Check suggestions against the guide and official documentation. An LLM's confidence is not proof that a disk selection, firmware write, or recovery command is correct. For inaccessible firmware settings or a machine that still cannot start after the documented recovery attempt, use Lenovo support or a qualified repair technician.

Keep recovery keys, private signing keys, and full raw trust exports private. Remove personal paths, serial numbers, and unrelated identifiers before sharing screenshots or logs. Partition identifiers may be needed to match paths during local analysis; keep them out of public posts.

## Secure Boot inventory is preparation

Successful reads establish configuration, not a supported enrollment method. Preserve the existing OEM/Microsoft trust and current revocations. Do not clear PK or restore factory keys simply because exports exist. The [signing guide](../signing/WALKTHROUGH.md) describes the unresolved enrollment and recovery requirements.

## Downloaded guides and historical records

Published release archives and explicitly historical snapshots retain their original text and checksums. Use the current online installation and signing guides for updated explanations. Historical requests to send a log are not a current requirement or a promise of maintainer support.
