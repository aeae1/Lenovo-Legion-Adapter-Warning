@echo -off
if "%1" == "" then
  echo Usage: disable-internal.nsh fsN: DISABLE
  echo Identify the internal Windows EFI volume again; numbers can change.
  exit /b 0
endif
if not exist %1\EFI\Microsoft\Boot\bootmgfw.efi then
  echo STOP_TARGET: the selected volume does not contain Windows Boot Manager.
  exit /b 1
endif
if not exist %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi then
  echo STOP_MISSING: there is no active helper at the internal project path.
  exit /b 1
endif
if exist %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.disabled then
  echo STOP_EXISTS: a disabled copy already exists. Nothing overwritten.
  exit /b 1
endif
if "%2" ne "DISABLE" then
  echo CHECK_ONLY: no files or saved entries were changed.
  exit /b 0
endif
mv %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.disabled
if %lasterror% ne 0 then
  echo STOP_RENAME: disabling failed. Check the error above.
  exit /b 1
endif
echo DISABLED_OK: the saved internal path cannot load this helper next boot.
echo The current boot's RAM patch lasts until shutdown.
echo Remove only the INTERNAL entry's current Option position shown below.
bcfg driver dump -v
exit /b 0
