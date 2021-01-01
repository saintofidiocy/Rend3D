@echo off
echo %1
comp\tasm -80 -b %1.z80 %1.bin
if errorlevel 1 goto ERRORS
comp\binpac8x.py %1.bin
goto DONE
:ERRORS
echo -- Errors were found!
:DONE
echo -- Done
echo ------------------------------------------
pause
echo.