c88 ABORT px %1
if errorlevel 1 goto stop

asm88 ACCESS %1
if errorlevel 1 goto stop

c88 ASIN px %1
if errorlevel 1 goto stop

c88 ATAN px %1
if errorlevel 1 goto stop

c88 ATAN2 px %1
if errorlevel 1 goto stop

c88 ATAN7 px %1
if errorlevel 1 goto stop

asm88 ATOL %1
if errorlevel 1 goto stop

c88 BSEARCH px %1
if errorlevel 1 goto stop

asm88 BUILTIN %1
if errorlevel 1 goto stop

asm88 CHDIR %1
if errorlevel 1 goto stop

c88 CHSIZE px %1
if errorlevel 1 goto stop

c88 CREAT3 px %1
if errorlevel 1 goto stop

c88 CTYPE px %1
if errorlevel 1 goto stop

asm88 DATES %1
if errorlevel 1 goto stop

c88 EXP px %1
if errorlevel 1 goto stop

c88 EXP10 px %1
if errorlevel 1 goto stop

asm88 EXPA %1
if errorlevel 1 goto stop

asm88 FABS %1
if errorlevel 1 goto stop

asm88 FINPUT %1
if errorlevel 1 goto stop

asm88 FLOAT %1
if errorlevel 1 goto stop

asm88 FLOAT7 %1
if errorlevel 1 goto stop

asm88 FLOOR %1
if errorlevel 1 goto stop

asm88 FLOOR7 %1
if errorlevel 1 goto stop

asm88 FOUTPUT %1
if errorlevel 1 goto stop

c88 FPRINTF px %1
if errorlevel 1 goto stop

c88 FREXP px %1
if errorlevel 1 goto stop

c88 FSCANF px %1
if errorlevel 1 goto stop

c88 FSTUFF px %1
if errorlevel 1 goto stop

c88 GETENV px %1
if errorlevel 1 goto stop

asm88 ISETUP %1
if errorlevel 1 goto stop

c88 ITOA px %1
if errorlevel 1 goto stop

c88 LDEXP px %1
if errorlevel 1 goto stop

asm88 LMOVE %1
if errorlevel 1 goto stop

c88 LOG px %1
if errorlevel 1 goto stop

asm88 LOGA %1
if errorlevel 1 goto stop

c88 LOG10 px %1
if errorlevel 1 goto stop

asm88 LTOA %1
if errorlevel 1 goto stop

asm88 MALLOC %1
if errorlevel 1 goto stop

asm88 MEMCHR %1
if errorlevel 1 goto stop

asm88 MEMCMP %1
if errorlevel 1 goto stop

asm88 MEMCPY %1
if errorlevel 1 goto stop

asm88 MEMICMP %1
if errorlevel 1 goto stop

asm88 MEMMOVE %1
if errorlevel 1 goto stop

asm88 MEMSET %1
if errorlevel 1 goto stop

c88 MODF px %1
if errorlevel 1 goto stop

c88 MOVERLAY px %1
if errorlevel 1 goto stop

asm88 MSVER2 %1
if errorlevel 1 goto stop

c88 MTAN px %1
if errorlevel 1 goto stop

asm88 MTANA %1
if errorlevel 1 goto stop

asm88 MULDIV %1
if errorlevel 1 goto stop

c88 NEWLIB px %1
if errorlevel 1 goto stop

c88 OVERLAY px %1
if errorlevel 1 goto stop

asm88 PEEKPOKE %1
if errorlevel 1 goto stop

c88 POW px %1
if errorlevel 1 goto stop

c88 PRINTF px %1
if errorlevel 1 goto stop

c88 PUTENV px %1
if errorlevel 1 goto stop

c88 QSORT px %1
if errorlevel 1 goto stop

c88 RAND px %1
if errorlevel 1 goto stop

c88 RERRNO px %1
if errorlevel 1 goto stop

c88 SCANF px %1
if errorlevel 1 goto stop

asm88 SETJMP %1
if errorlevel 1 goto stop

c88 SIN px %1
if errorlevel 1 goto stop

c88 SPRINTF px %1
if errorlevel 1 goto stop

asm88 SQRT %1
if errorlevel 1 goto stop

asm88 STRCAT %1
if errorlevel 1 goto stop

asm88 STRCHR %1
if errorlevel 1 goto stop

asm88 STRCMP %1
if errorlevel 1 goto stop

asm88 STRCPY %1
if errorlevel 1 goto stop

c88 STRDUP px %1
if errorlevel 1 goto stop

asm88 STRINGI %1
if errorlevel 1 goto stop

asm88 STRLEN %1
if errorlevel 1 goto stop

c88 STRLWR px %1
if errorlevel 1 goto stop

asm88 STRRCHR %1
if errorlevel 1 goto stop

c88 STRREV px %1
if errorlevel 1 goto stop

c88 STRSET px %1
if errorlevel 1 goto stop

c88 STRSTR px %1
if errorlevel 1 goto stop

c88 STRTOD px %1
if errorlevel 1 goto stop

c88 STRTOK px %1
if errorlevel 1 goto stop

c88 STRTOL px %1
if errorlevel 1 goto stop

c88 STRUPR px %1
if errorlevel 1 goto stop

c88 SYSTEM px %1
if errorlevel 1 goto stop

c88 TAN px %1
if errorlevel 1 goto stop

asm88 UMULDIV %1
if errorlevel 1 goto stop

asm88 UPPER %1
if errorlevel 1 goto stop

asm88 _ADX %1
if errorlevel 1 goto stop

c88 _AINTRND px %1
if errorlevel 1 goto stop

c88 _DOPRINT px %1
if errorlevel 1 goto stop

c88 _INTRND px %1
if errorlevel 1 goto stop

c88 _PI px %1
if errorlevel 1 goto stop

c88 _PI_2 px %1
if errorlevel 1 goto stop

if .%1.==.. goto make_small

lib88 -fmakelib.lst -obmylib.s
lib88 -fmakelib7.lst -obmylib7.s
goto stop

:make_small
lib88 -fmakelib.lst -omylib.s
lib88 -fmakelib7.lst -omylib7.s

:stop
