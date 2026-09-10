# Secure Boot menu observations

Reviewed from the owner's GKCN65WW firmware screenshots on September 10, 2026.

| Visible field | Displayed value or action |
| --- | --- |
| Secure Boot | Disabled |
| Secure Boot Status | Disabled |
| Platform Mode | User Mode |
| Secure Boot Mode | Standard |
| Reset to Setup Mode | Help text says it clears PK, disables Secure Boot, and enters Setup Mode |
| Restore Factory Keys | Help text says it restores PK, KEK, db, and dbx to factory defaults |

No image-approval or certificate-enrollment control is visible in the supplied screenshots. Their absence from these screens does not establish absence from the entire firmware. Conversely, enrollment strings in a firmware module do not prove an accessible Lenovo menu.

The screenshots support further read-only inventory and implementation research. They do not justify clearing keys or treating a reset action as an append-only certificate enrollment operation. No key change is recorded here. AMD PSP controls are separate from this helper's trust requirement.
