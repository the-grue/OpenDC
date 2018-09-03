echo off

del *.obj

c88 ABORT px b m %1
if errorlevel 1 goto stop

asm88 ACCESS b m %1
if errorlevel 1 goto stop

c88 ASIN px b m %1
if errorlevel 1 goto stop

c88 ATAN px b m %1
if errorlevel 1 goto stop

c88 ATAN2 px b m %1
if errorlevel 1 goto stop

c88 ATAN7 px b m %1
if errorlevel 1 goto stop

asm88 ATOL b m %1
if errorlevel 1 goto stop

c88 BSEARCH px b m %1
if errorlevel 1 goto stop

asm88 BUILTIN b m %1
if errorlevel 1 goto stop

asm88 CHDIR b m %1
if errorlevel 1 goto stop

c88 CHSIZE px b m %1
if errorlevel 1 goto stop

c88 CREAT3 px b m %1
if errorlevel 1 goto stop

c88 CTYPE px b m %1
if errorlevel 1 goto stop

asm88 DATES b m %1
if errorlevel 1 goto stop

c88 ECVT px b m %1
if errorlevel 1 goto stop

c88 EXP px b m %1
if errorlevel 1 goto stop

c88 EXP10 px b m %1
if errorlevel 1 goto stop

asm88 EXPA b m %1
if errorlevel 1 goto stop

asm88 FABS b m %1
if errorlevel 1 goto stop

asm88 FINPUT b m %1
if errorlevel 1 goto stop

asm88 FLOAT b m %1
if errorlevel 1 goto stop

asm88 FLOAT7 b m %1
if errorlevel 1 goto stop

asm88 FLOOR b m %1
if errorlevel 1 goto stop

asm88 FLOOR7 b m %1
if errorlevel 1 goto stop

asm88 FOUTPUT b m %1
if errorlevel 1 goto stop

c88 FPRINTF px b m %1
if errorlevel 1 goto stop

c88 FREXP px b m %1
if errorlevel 1 goto stop

c88 FSCANF px b m %1
if errorlevel 1 goto stop

c88 FSTUFF px b m %1
if errorlevel 1 goto stop

c88 GETENV px b m %1
if errorlevel 1 goto stop

asm88 ISETUP b m %1
if errorlevel 1 goto stop

c88 ITOA px b m %1
if errorlevel 1 goto stop

c88 LDEXP px b m %1
if errorlevel 1 goto stop

asm88 LMOVE b m %1
if errorlevel 1 goto stop

c88 LOG px b m %1
if errorlevel 1 goto stop

asm88 LOGA b m %1
if errorlevel 1 goto stop

c88 LOG10 px b m %1
if errorlevel 1 goto stop

asm88 LTOA b m %1
if errorlevel 1 goto stop

asm88 MALLOC b m %1
if errorlevel 1 goto stop

asm88 MEMCHR b m %1
if errorlevel 1 goto stop

asm88 MEMCMP b m %1
if errorlevel 1 goto stop

asm88 MEMCPY b m %1
if errorlevel 1 goto stop

asm88 MEMICMP b m %1
if errorlevel 1 goto stop

asm88 MEMMOVE b m %1
if errorlevel 1 goto stop

asm88 MEMSET b m %1
if errorlevel 1 goto stop

c88 MODF px b m %1
if errorlevel 1 goto stop

rem c88 MOVERLAY px b m %1
rem MOVERLAY not in big library
if errorlevel 1 goto stop

asm88 MSVER2 b m %1
if errorlevel 1 goto stop

c88 MTAN px b m %1
if errorlevel 1 goto stop

asm88 MTANA b m %1
if errorlevel 1 goto stop

asm88 MULDIV b m %1
if errorlevel 1 goto stop

c88 NEWLIB px b m %1
if errorlevel 1 goto stop

rem c88 OVERLAY px b m %1
rem OVERLAY not in big library
if errorlevel 1 goto stop

asm88 PEEKPOKE b m %1
if errorlevel 1 goto stop

c88 POW px b m %1
if errorlevel 1 goto stop

c88 PRINTF px b m %1
if errorlevel 1 goto stop

c88 PUTENV px b m %1
if errorlevel 1 goto stop

c88 QSORT px b m %1
if errorlevel 1 goto stop

c88 RAND px b m %1
if errorlevel 1 goto stop

c88 RERRNO px b m %1
if errorlevel 1 goto stop

c88 SCANF px b m %1
if errorlevel 1 goto stop

asm88 SETJMP b m %1
if errorlevel 1 goto stop

c88 SIN px b m %1
if errorlevel 1 goto stop

c88 SPRINTF px b m %1
if errorlevel 1 goto stop

asm88 SQRT b m %1
if errorlevel 1 goto stop

asm88 STRCAT b m %1
if errorlevel 1 goto stop

asm88 STRCHR b m %1
if errorlevel 1 goto stop

asm88 STRCMP b m %1
if errorlevel 1 goto stop

asm88 STRCPY b m %1
if errorlevel 1 goto stop

c88 STRDUP px b m %1
if errorlevel 1 goto stop

asm88 STRINGI b m %1
if errorlevel 1 goto stop

asm88 STRLEN b m %1
if errorlevel 1 goto stop

c88 STRLWR px b m %1
if errorlevel 1 goto stop

asm88 STRRCHR b m %1
if errorlevel 1 goto stop

c88 STRREV px b m %1
if errorlevel 1 goto stop

c88 STRSET px b m %1
if errorlevel 1 goto stop

c88 STRSTR px b m %1
if errorlevel 1 goto stop

c88 STRTOD px b m %1
if errorlevel 1 goto stop

c88 STRTOK px b m %1
if errorlevel 1 goto stop

c88 STRTOL px b m %1
if errorlevel 1 goto stop

c88 STRUPR px b m %1
if errorlevel 1 goto stop

c88 SYSTEM px b m %1
if errorlevel 1 goto stop

c88 TAN px b m %1
if errorlevel 1 goto stop

asm88 UMULDIV b m %1
if errorlevel 1 goto stop

asm88 UPPER b m %1
if errorlevel 1 goto stop

asm88 _ADX b m %1
if errorlevel 1 goto stop

c88 _AINTRND px b m %1
if errorlevel 1 goto stop

c88 _DOPRINT px b m %1
if errorlevel 1 goto stop

c88 _DOSCAN px b m %1
if errorlevel 1 goto stop

c88 _INTRND px b m %1
if errorlevel 1 goto stop

c88 _PI px b m %1
if errorlevel 1 goto stop

c88 _PI_2 px b m %1
if errorlevel 1 goto stop

rem lib88 -fmakeblib.lst -obmylib.s
rem lib88 -fmakblib7.lst -obmylib7.s

echo on
echo BUILDING BC88.LIB
echo off
call bc88lib.bat

echo on
echo BUILDING BC887.LIB
echo off
bc887lib.bat

:stop
