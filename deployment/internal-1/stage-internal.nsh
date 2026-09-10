@echo -off
if "%1" == "" then
  echo Usage: stage-internal.nsh fsN: COPY
  echo fsN: must be the internal Windows EFI partition identified in the guide.
  echo Without COPY, this script only checks paths and displays DriverOrder.
  exit /b 0
endif
if not exist LenovoWarning-v4-Driver.efi then
  echo STOP_SOURCE: run from the USB folder containing the verified V4 helper.
  exit /b 1
endif
if not exist %1\EFI\Microsoft\Boot\bootmgfw.efi then
  echo STOP_TARGET: the selected volume does not contain Windows Boot Manager.
  exit /b 1
endif
echo Target: %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
echo Confirm in the verbose mapping list that this is your internal disk.
bcfg driver dump -v
if "%2" ne "COPY" then
  echo CHECK_ONLY: no files or saved driver entries were changed.
  exit /b 0
endif
if exist %1\EFI\LenovoAdapterWarningV4 then
  echo STOP_EXISTS: the project folder already exists. Nothing overwritten.
  echo Do not run COPY again. Follow the existing-installation note in the guide.
  exit /b 1
endif
mkdir %1\EFI\LenovoAdapterWarningV4
if %lasterror% ne 0 then
  echo STOP_MKDIR: could not create the project folder. No entry was added.
  exit /b 1
endif
cp LenovoWarning-v4-Driver.efi %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
if %lasterror% ne 0 then
  echo STOP_COPY: copy failed. Do not register the driver.
  echo A partial project folder may remain. No entry was added.
  exit /b 1
endif
comp LenovoWarning-v4-Driver.efi %1\EFI\LenovoAdapterWarningV4\LenovoWarning-v4-Driver.efi
if %lasterror% ne 0 then
  echo STOP_COMPARE: verification failed. Do not register the driver.
  echo The project folder remains for inspection. No entry was added.
  exit /b 1
endif
echo STAGED_OK: the internal copy matches the USB helper byte for byte.
echo No driver entry was added. Follow the guide to replace the USB TEST entry.
exit /b 0
