@echo off
for %%v in (*.z80) do call ASM "%%~nv"
exit
