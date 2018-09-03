echo off
c88 toobj 
if errorlevel 1 goto stop
bind toobj -otoobj
if errorlevel 1 goto stop
c88 dumpo
if errorlevel 1 goto stop
bind dumpo -odumpobj
if errorlevel 1 goto stop
:stop
