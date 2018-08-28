c88 lib88 %1
if errorlevel 1 goto stop

c88 squish %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind lib88 -omy_lib88

bind squish -omysquish
goto stop

:generate_chk
bind lib88 -omy_lib88 -c

bind squish -omysquish -c

:stop
