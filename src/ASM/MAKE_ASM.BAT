echo off
del my_asm88.exe >nul
del chain.o >nul
asm88 chain
if errorlevel 1 goto stop
if not exist chain.o goto stop

del asm88.o >nul
c88 asm88 %1
if errorlevel 1 goto stop
if not exist asm88.o goto stop

del asm2.o >nul
c88 asm2 %1
if errorlevel 1 goto stop
if not exist asm2.o goto stop

del asm3.o >nul
c88 asm3 %1
if errorlevel 2 goto stop
if not exist asm3.o goto stop

del asm4.o >nul
c88 asm4 %1
if errorlevel 1 goto stop
if not exist asm4.o goto stop

del asm5.o >nul
c88 asm5 %1
if errorlevel 1 goto stop
if not exist asm5.o goto stop

if .%1.==.c. goto generate_chk

bind chain asm88 asm2 asm3 asm4 asm5 -omy_asm88
goto stop

:generate_chk
bind chain asm88 asm2 asm3 asm4 asm5 -omy_asm88 -c

:stop
