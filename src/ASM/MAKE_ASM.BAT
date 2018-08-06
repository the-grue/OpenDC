asm88 chain
if errorlevel 1 goto stop

c88 asm88 %1
if errorlevel 1 goto stop

c88 asm2 %1
if errorlevel 1 goto stop

c88 asm3 %1
if errorlevel 2 goto stop

c88 asm4 %1
if errorlevel 1 goto stop

c88 asm5 %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind chain asm88 asm2 asm3 asm4 asm5 -omy_asm88
goto stop

:generate_chk
bind chain asm88 asm2 asm3 asm4 asm5 -omy_asm88 -c

:stop
