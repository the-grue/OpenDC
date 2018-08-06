asm88 chain
if errorlevel 1 goto stop

c88 c88 %1
if errorlevel 1 goto stop

rem
rem  ANSI compiler, need to add "#pragma extended" via command line (px)
rem  to turn on inline assembly
rem
c88 c2 %1 px
if errorlevel 1 goto stop

c88 c3 %1
if errorlevel 1 goto stop

c88 c4 %1
if errorlevel 1 goto stop

c88 c5 %1
if errorlevel 1 goto stop

c88 c6 %1
if errorlevel 1 goto stop

rem
rem  ANSI compiler, need to add "#pragma extended" via command line (px)
rem  to turn on inline assembly
rem
c88 c7 %1 px
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind chain c88 c2 c3 c4 c5 c6 c7 -omy_c88
goto stop

:generate_chk
bind chain c88 c2 c3 c4 c5 c6 c7 -omy_c88 -c

:stop
