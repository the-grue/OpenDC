echo off

asm88 _lmov
if errorlevel 1 goto stop

asm88 _lcmp
if errorlevel 1 goto stop

asm88 _lcpy
if errorlevel 1 goto stop

c88 nbind %1
if errorlevel 1 goto stop

if .%1.==.c. goto generate_chk

bind nbind _lmov _lcmp _lcpy -omynbind
goto stop

bind nbind _lmov _lcmp _lcpy -omynbind -c

:stop
