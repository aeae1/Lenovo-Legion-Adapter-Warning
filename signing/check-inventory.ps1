#Requires -Version 5.1
$ErrorActionPreference = 'Stop'
$source = Join-Path $PSScriptRoot 'Read-SecureBootState.ps1'
$tokens = $null
$errors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile($source, [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw 'Inventory script has syntax errors.' }
$commands = $ast.FindAll({ param($n) $n -is [System.Management.Automation.Language.CommandAst] }, $true)
foreach ($command in $commands) {
    if ($command.GetCommandName() -match '^(Set-SecureBootUEFI|Format-SecureBootUEFI|mountvol|bcdedit|Clear-Tpm)$') {
        throw ('Unexpected firmware or boot mutation: ' + $command.GetCommandName())
    }
}
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('v4-inventory-test-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
Copy-Item -LiteralPath $source -Destination $testRoot
$global:v4FailDb = $false
$global:v4Unsupported = $false
function Confirm-SecureBootUEFI {
    [CmdletBinding()] param()
    if ($global:v4Unsupported) { throw 'Synthetic unsupported firmware' }
    return $false
}
function Get-SecureBootUEFI {
    [CmdletBinding()] param([string]$Name)
    if ($Name -eq 'db' -and $global:v4FailDb) { throw 'Synthetic db read failure' }
    if ($Name -eq 'PKDefault') { throw 'Synthetic absent optional default' }
    [byte[]]$bytes = @(1, 2, 3, 4)
    if ($Name -eq 'SecureBoot' -or $Name -eq 'SetupMode') { $bytes = @(0) }
    return [pscustomobject]@{ Bytes = $bytes; Attributes = 'TEST_ONLY' }
}
try {
    $scriptPath = Join-Path $testRoot 'Read-SecureBootState.ps1'
    & $scriptPath
    $reports = @(Get-ChildItem (Join-Path $testRoot 'private-results') -Filter summary.json -Recurse)
    if ($reports.Count -ne 1) { throw 'Expected one initial report.' }
    $first = Get-Content -LiteralPath $reports[0].FullName -Raw | ConvertFrom-Json
    if ($first.secure_boot_enabled -or $first.firmware_changed) { throw 'Incorrect state flags.' }
    if (($first.variables | Where-Object name -eq 'SetupMode').value -ne 0) { throw 'Incorrect mode.' }
    $global:v4FailDb = $true
    & $scriptPath
    $reports = @(Get-ChildItem (Join-Path $testRoot 'private-results') -Filter summary.json -Recurse)
    if ($reports.Count -ne 2) { throw 'Second run overwrote an existing report.' }
    $parsed = @($reports | ForEach-Object { Get-Content -LiteralPath $_.FullName -Raw | ConvertFrom-Json })
    $failedDb = @($parsed | Where-Object { ($_.variables | Where-Object name -eq 'db').read_status -eq 'unavailable' })
    if ($failedDb.Count -ne 1) { throw 'db read failure was not preserved.' }
    if (-not ($parsed | Where-Object { ($_.variables | Where-Object name -eq 'db').read_status -eq 'read' })) {
        throw 'Successful first report was lost.'
    }
    $global:v4Unsupported = $true
    $threw = $false
    try { & $scriptPath } catch { $threw = $true }
    if (-not $threw) { throw 'Unsupported platform incorrectly reported success.' }
    if (@(Get-ChildItem (Join-Path $testRoot 'private-results') -Filter summary.json -Recurse).Count -ne 2) {
        throw 'Unsupported firmware produced a misleading report.'
    }
    Write-Host 'PASS inventory syntax, state reads, missing-variable reporting, separate runs, and unsupported-platform refusal.'
    Write-Host 'Mock queries only: no real firmware was read or changed by this test.'
} finally {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
    Remove-Variable v4FailDb, v4Unsupported -Scope Global
}
