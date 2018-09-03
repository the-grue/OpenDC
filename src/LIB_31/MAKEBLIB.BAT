echo off

c88 ABORT px b %1
if errorlevel 1 goto stop

asm88 ACCESS b %1
if errorlevel 1 goto stop

c88 ASIN px b %1
if errorlevel 1 goto stop

c88 ATAN px b %1
if errorlevel 1 goto stop

c88 ATAN2 px b %1
if errorlevel 1 goto stop

c88 ATAN7 px b %1
if errorlevel 1 goto stop

asm88 ATOL b %1
if errorlevel 1 goto stop

c88 BSEARCH px b %1
if errorlevel 1 goto stop

asm88 BUILTIN b %1
if errorlevel 1 goto stop

asm88 CHDIR b %1
if errorlevel 1 goto stop

c88 CHSIZE px b %1
if errorlevel 1 goto stop

c88 CREAT3 px b %1
if errorlevel 1 goto stop

c88 CTYPE px b %1
if errorlevel 1 goto stop

asm88 DATES b %1
if errorlevel 1 goto stop

c88 ECVT px b %1
if errorlevel 1 goto stop

c88 EXP px b %1
if errorlevel 1 goto stop

c88 EXP10 px b %1
if errorlevel 1 goto stop

asm88 EXPA b %1
if errorlevel 1 goto stop

asm88 FABS b %1
if errorlevel 1 goto stop

asm88 FINPUT b %1
if errorlevel 1 goto stop

asm88 FLOAT b %1
if errorlevel 1 goto stop

asm88 FLOAT7 b %1
if errorlevel 1 goto stop

asm88 FLOOR b %1
if errorlevel 1 goto stop

asm88 FLOOR7 b %1
if errorlevel 1 goto stop

asm88 FOUTPUT b %1
if errorlevel 1 goto stop

c88 FPRINTF px b %1
if errorlevel 1 goto stop

c88 FREXP px b %1
if errorlevel 1 goto stop

c88 FSCANF px b %1
if errorlevel 1 goto stop

c88 FSTUFF px b %1
if errorlevel 1 goto stop

c88 GETENV px b %1
if errorlevel 1 goto stop

asm88 ISETUP b %1
if errorlevel 1 goto stop

c88 ITOA px b %1
if errorlevel 1 goto stop

c88 LDEXP px b %1
if errorlevel 1 goto stop

asm88 LMOVE b %1
if errorlevel 1 goto stop

c88 LOG px b %1
if errorlevel 1 goto stop

asm88 LOGA b %1
if errorlevel 1 goto stop

c88 LOG10 px b %1
if errorlevel 1 goto stop

asm88 LTOA b %1
if errorlevel 1 goto stop

asm88 MALLOC b %1
if errorlevel 1 goto stop

asm88 MEMCHR b %1
if errorlevel 1 goto stop

asm88 MEMCMP b %1
if errorlevel 1 goto stop

asm88 MEMCPY b %1
if errorlevel 1 goto stop

asm88 MEMICMP b %1
if errorlevel 1 goto stop

asm88 MEMMOVE b %1
if errorlevel 1 goto stop

asm88 MEMSET b %1
if errorlevel 1 goto stop

c88 MODF px b %1
if errorlevel 1 goto stop

rem c88 MOVERLAY px b %1
rem MOVERLAY not in big library
if errorlevel 1 goto stop

asm88 MSVER2 b %1
if errorlevel 1 goto stop

c88 MTAN px b %1
if errorlevel 1 goto stop

asm88 MTANA b %1
if errorlevel 1 goto stop

asm88 MULDIV b %1
if errorlevel 1 goto stop

c88 NEWLIB px b %1
if errorlevel 1 goto stop

rem c88 OVERLAY px b %1
rem OVERLAY not in big library
if errorlevel 1 goto stop

asm88 PEEKPOKE b %1
if errorlevel 1 goto stop

c88 POW px b %1
if errorlevel 1 goto stop

c88 PRINTF px b %1
if errorlevel 1 goto stop

c88 PUTENV px b %1
if errorlevel 1 goto stop

c88 QSORT px b %1
if errorlevel 1 goto stop

c88 RAND px b %1
if errorlevel 1 goto stop

c88 RERRNO px b %1
if errorlevel 1 goto stop

c88 SCANF px b %1
if errorlevel 1 goto stop

asm88 SETJMP b %1
if errorlevel 1 goto stop

c88 SIN px b %1
if errorlevel 1 goto stop

c88 SPRINTF px b %1
if errorlevel 1 goto stop

asm88 SQRT b %1
if errorlevel 1 goto stop

asm88 STRCAT b %1
if errorlevel 1 goto stop

asm88 STRCHR b %1
if errorlevel 1 goto stop

asm88 STRCMP b %1
if errorlevel 1 goto stop

asm88 STRCPY b %1
if errorlevel 1 goto stop

c88 STRDUP px b %1
if errorlevel 1 goto stop

asm88 STRINGI b %1
if errorlevel 1 goto stop

asm88 STRLEN b %1
if errorlevel 1 goto stop

c88 STRLWR px b %1
if errorlevel 1 goto stop

asm88 STRRCHR b %1
if errorlevel 1 goto stop

c88 STRREV px b %1
if errorlevel 1 goto stop

c88 STRSET px b %1
if errorlevel 1 goto stop

c88 STRSTR px b %1
if errorlevel 1 goto stop

c88 STRTOD px b %1
if errorlevel 1 goto stop

c88 STRTOK px b %1
if errorlevel 1 goto stop

c88 STRTOL px b %1
if errorlevel 1 goto stop

c88 STRUPR px b %1
if errorlevel 1 goto stop

c88 SYSTEM px b %1
if errorlevel 1 goto stop

c88 TAN px b %1
if errorlevel 1 goto stop

asm88 UMULDIV b %1
if errorlevel 1 goto stop

asm88 UPPER b %1
if errorlevel 1 goto stop

asm88 _ADX b %1
if errorlevel 1 goto stop

c88 _AINTRND px b %1
if errorlevel 1 goto stop

c88 _DOPRINT px b %1
if errorlevel 1 goto stop

c88 _DOSCAN px b %1
if errorlevel 1 goto stop

c88 _INTRND px b %1
if errorlevel 1 goto stop

c88 _PI px b %1
if errorlevel 1 goto stop

c88 _PI_2 px b %1
if errorlevel 1 goto stop

lib88 -fmakeblib.lst -obmylib.s
lib88 -fmakblib7.lst -obmylib7.s

:stop
