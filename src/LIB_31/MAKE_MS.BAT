echo off

del *.obj

c88 ABORT px m %1
if errorlevel 1 goto stop

asm88 ACCESS m %1
if errorlevel 1 goto stop

c88 ASIN px m %1
if errorlevel 1 goto stop

c88 ATAN px m %1
if errorlevel 1 goto stop

c88 ATAN2 px m %1
if errorlevel 1 goto stop

c88 ATAN7 px m %1
if errorlevel 1 goto stop

asm88 ATOL m %1
if errorlevel 1 goto stop

c88 BSEARCH px m %1
if errorlevel 1 goto stop

asm88 BUILTIN m %1
if errorlevel 1 goto stop

asm88 CHDIR m %1
if errorlevel 1 goto stop

c88 CHSIZE px m %1
if errorlevel 1 goto stop

c88 CREAT3 px m %1
if errorlevel 1 goto stop

c88 CTYPE px m %1
if errorlevel 1 goto stop

asm88 DATES m %1
if errorlevel 1 goto stop

c88 ECVT px m %1
if errorlevel 1 goto stop

c88 EXP px m %1
if errorlevel 1 goto stop

c88 EXP10 px m %1
if errorlevel 1 goto stop

asm88 EXPA m %1
if errorlevel 1 goto stop

asm88 FABS m %1
if errorlevel 1 goto stop

asm88 FINPUT m %1
if errorlevel 1 goto stop

asm88 FLOAT m %1
if errorlevel 1 goto stop

asm88 FLOAT7 m %1
if errorlevel 1 goto stop

asm88 FLOOR m %1
if errorlevel 1 goto stop

asm88 FLOOR7 m %1
if errorlevel 1 goto stop

asm88 FOUTPUT m %1
if errorlevel 1 goto stop

c88 FPRINTF px m %1
if errorlevel 1 goto stop

c88 FREXP px m %1
if errorlevel 1 goto stop

c88 FSCANF px m %1
if errorlevel 1 goto stop

c88 FSTUFF px m %1
if errorlevel 1 goto stop

c88 GETENV px m %1
if errorlevel 1 goto stop

asm88 ISETUP m %1
if errorlevel 1 goto stop

c88 ITOA px m %1
if errorlevel 1 goto stop

c88 LDEXP px m %1
if errorlevel 1 goto stop

asm88 LMOVE m %1
if errorlevel 1 goto stop

c88 LOG px m %1
if errorlevel 1 goto stop

asm88 LOGA m %1
if errorlevel 1 goto stop

c88 LOG10 px m %1
if errorlevel 1 goto stop

asm88 LTOA m %1
if errorlevel 1 goto stop

asm88 MALLOC m %1
if errorlevel 1 goto stop

asm88 MEMCHR m %1
if errorlevel 1 goto stop

asm88 MEMCMP m %1
if errorlevel 1 goto stop

asm88 MEMCPY m %1
if errorlevel 1 goto stop

asm88 MEMICMP m %1
if errorlevel 1 goto stop

asm88 MEMMOVE m %1
if errorlevel 1 goto stop

asm88 MEMSET m %1
if errorlevel 1 goto stop

c88 MODF px m %1
if errorlevel 1 goto stop

c88 MOVERLAY px m %1
if errorlevel 1 goto stop

asm88 MSVER2 m %1
if errorlevel 1 goto stop

c88 MTAN px m %1
if errorlevel 1 goto stop

asm88 MTANA m %1
if errorlevel 1 goto stop

asm88 MULDIV m %1
if errorlevel 1 goto stop

c88 NEWLIB px m %1
if errorlevel 1 goto stop

c88 OVERLAY px m %1
if errorlevel 1 goto stop

asm88 PEEKPOKE m %1
if errorlevel 1 goto stop

c88 POW px m %1
if errorlevel 1 goto stop

c88 PRINTF px m %1
if errorlevel 1 goto stop

c88 PUTENV px m %1
if errorlevel 1 goto stop

c88 QSORT px m %1
if errorlevel 1 goto stop

c88 RAND px m %1
if errorlevel 1 goto stop

c88 RERRNO px m %1
if errorlevel 1 goto stop

c88 SCANF px m %1
if errorlevel 1 goto stop

asm88 SETJMP m %1
if errorlevel 1 goto stop

c88 SIN px m %1
if errorlevel 1 goto stop

c88 SPRINTF px m %1
if errorlevel 1 goto stop

asm88 SQRT m %1
if errorlevel 1 goto stop

asm88 STRCAT m %1
if errorlevel 1 goto stop

asm88 STRCHR m %1
if errorlevel 1 goto stop

asm88 STRCMP m %1
if errorlevel 1 goto stop

asm88 STRCPY m %1
if errorlevel 1 goto stop

c88 STRDUP px m %1
if errorlevel 1 goto stop

asm88 STRINGI m %1
if errorlevel 1 goto stop

asm88 STRLEN m %1
if errorlevel 1 goto stop

c88 STRLWR px m %1
if errorlevel 1 goto stop

asm88 STRRCHR m %1
if errorlevel 1 goto stop

c88 STRREV px m %1
if errorlevel 1 goto stop

c88 STRSET px m %1
if errorlevel 1 goto stop

c88 STRSTR px m %1
if errorlevel 1 goto stop

c88 STRTOD px m %1
if errorlevel 1 goto stop

c88 STRTOK px m %1
if errorlevel 1 goto stop

c88 STRTOL px m %1
if errorlevel 1 goto stop

c88 STRUPR px m %1
if errorlevel 1 goto stop

c88 SYSTEM px m %1
if errorlevel 1 goto stop

c88 TAN px m %1
if errorlevel 1 goto stop

asm88 UMULDIV m %1
if errorlevel 1 goto stop

asm88 UPPER m %1
if errorlevel 1 goto stop

asm88 _ADX m %1
if errorlevel 1 goto stop

c88 _AINTRND px m %1
if errorlevel 1 goto stop

c88 _DOPRINT px m %1
if errorlevel 1 goto stop

c88 _DOSCAN px m %1
if errorlevel 1 goto stop

c88 _INTRND px m %1
if errorlevel 1 goto stop

c88 _PI px m %1
if errorlevel 1 goto stop

c88 _PI_2 px m %1
if errorlevel 1 goto stop

rem lib88 -fmakelib.lst -omylib.s
rem lib88 -fmakelib7.lst -omylib7.s

echo on
echo BUILDING C88.LIB
echo off
call C88LIB.BAT

echo on
echo BUILDING BC88.LIB
echo off
call C887LIB.BAT

:stop
