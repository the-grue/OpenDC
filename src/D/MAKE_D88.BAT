echo off

c88 d88 -px %1
if errorlevel 1 goto stop

c88 tokit %1
if errorlevel 1 goto stop

c88 gline %1
if errorlevel 1 goto stop

c88 go %1
if errorlevel 1 goto stop

asm88 d88a 
if errorlevel 1 goto stop

asm88 dis 
if errorlevel 1 goto stop

asm88 pcfast 
if errorlevel 1 goto stop

asm88 flip 
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind d88 tokit gline go d88a dis pcfast flip -omy_d88
goto stop

:generate_chk
bind d88 tokit gline go d88a dis pcfast flip -omy_d88 -c

:stop
