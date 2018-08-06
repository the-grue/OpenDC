c88 lib88 %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind lib88 -omy_lib88
goto stop

:generate_chk
bind lib88 -omy_lib88 -c

:stop
