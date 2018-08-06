asm88 chain
if errorlevel 1 goto stop

rem
rem  ANSI compiler, need to add "#pragma extended" via command line (px)
rem  to turn on inline assembly
rem
c88 gen %1 px
if errorlevel 1 goto stop

c88 gen2 %1
if errorlevel 1 goto stop

c88 gen3 %1
if errorlevel 1 goto stop

c88 gen4 %1
if errorlevel 1 goto stop

c88 gen5 %1
if errorlevel 1 goto stop

c88 gen6 %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind chain gen gen2 gen3 gen4 gen5 gen6 -omy_gen
goto stop

:generate_chk
bind chain gen gen2 gen3 gen4 gen5 gen6 -omy_gen -c

:stop
