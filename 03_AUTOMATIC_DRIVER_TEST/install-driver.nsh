@echo -off
if not exist LenovoWarning-v4-Driver.efi then
  echo STOP: change to the USB root containing LenovoWarning-v4-Driver.efi first.
  goto END
endif
bcfg driver dump -v
if "%1" ne "INSTALL" then
  echo Review the current list above. Remove old Lenovo V2/V3/V4 test entries first.
  echo After the manual test succeeds, run: install-driver.nsh INSTALL
  goto END
endif
bcfg driver add 0 LenovoWarning-v4-Driver.efi "Lenovo USB-C Warning Patch v4 TEST"
bcfg driver dump -v
echo Verify one V4 TEST entry exists and no command above reported an error.
echo The script does not reboot. Follow WALKTHROUGH.md for the next boot.
:END
