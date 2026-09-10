#Requires -Version 5.1
#Requires -RunAsAdministrator
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'

# Reads firmware through Microsoft's query cmdlets. Output is ordinary local files.
# No firmware-variable setter, enrollment, key reset, ESP mount, or driver change.
Get-Command Get-SecureBootUEFI, Confirm-SecureBootUEFI -ErrorAction Stop | Out-Null
$enabled = Confirm-SecureBootUEFI -ErrorAction Stop
$suffix = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
$out = Join-Path $PSScriptRoot ('private-results\SecureBoot-' + $suffix)
New-Item -ItemType Directory -Path $out -ErrorAction Stop | Out-Null
$records = @()
foreach ($name in @('SecureBoot', 'SetupMode', 'PK', 'KEK', 'db', 'dbx', 'PKDefault', 'KEKDefault', 'dbDefault', 'dbxDefault')) {
    $variable = $null
    try {
        $variable = Get-SecureBootUEFI -Name $name -ErrorAction Stop
    } catch {
        $records += [ordered]@{ name = $name; read_status = 'unavailable'; error_type = $_.Exception.GetType().Name }
        continue
    }
    [byte[]]$bytes = $variable.Bytes
    if ($null -eq $bytes) { throw ('Query returned no byte array for ' + $name) }
    $file = Join-Path $out ($name + '.bin')
    [System.IO.File]::WriteAllBytes($file, $bytes)
    $record = [ordered]@{
        name = $name
        read_status = 'read'
        byte_count = $bytes.Length
        attributes = [string]$variable.Attributes
        sha256 = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash
    }
    if (($name -eq 'SecureBoot' -or $name -eq 'SetupMode') -and $bytes.Length -eq 1) {
        $record['value'] = [int]$bytes[0]
    }
    $records += $record
}
$summary = [ordered]@{
    format = 'lenovo-v4-secure-boot-inventory-1'
    secure_boot_enabled = [bool]$enabled
    firmware_changed = $false
    variables = $records
}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $out 'summary.json') -Encoding UTF8
Write-Host 'Finished reading Secure Boot state. No firmware settings were changed.'
Write-Host ('Results: ' + $out)
Write-Host 'Send summary.json for review. Keep the .bin exports private.'
Write-Host 'These are inventory exports, not a guaranteed one-click recovery backup.'
