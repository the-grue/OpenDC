c88 bind %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind bind -omy_bind
goto stop

bind bind -omy_bind -c

:stop
