echo off

mkdir bin

echo Building asm88.exe

cd asm
call make_asm.bat
if not exist my_asm88.exe goto stop
copy my_asm88.exe ..\bin\asm88.exe
cd ..

echo Building bind.exe

cd bind
call makebind.bat
if not exist my_bind.exe goto stop
copy my_bind.exe ..\bin\bind.exe
cd ..

echo Building c88.exe

cd c
call make_c88.bat
if not exist my_c88.exe goto stop
cd ..

echo Building d88.exe

cd d
call make_d88.bat
if not exist my_d88.exe goto stop
copy my_d88.exe ..\bin\d88.exe
cd ..

echo Building gen.exe

cd gen
call make_gen.bat
if not exist my_gen.exe goto stop
copy my_gen.exe ..\bin\gen.exe
cd ..

@REM update compiler back-end and front-end at the same time
copy c\my_c88.exe bin\c88.exe

echo Building lib88.exe

cd lib88
call make_l88.bat
if not exist my_lib88.exe goto stop
if not exist mysquish.exe goto stop
copy my_lib88.exe ..\bin\lib88.exe
copy mysquish.exe ..\bin\squish.exe
cd ..

echo Building nbind.exe

cd nbind
call make_nb.bat
if not exist mynbind.exe goto stop
copy mynbind.exe ..\bin\nbind.exe
cd ..

echo Building bbind.exe

cd bbind
call make_bb.bat
if not exist my_bbind.exe goto stop
copy my_bbind.exe ..\bin\bbind.exe
cd ..

echo Building others

cd other
copy llink.bat ..\bin\llink.bat
call other.bat
if not exist toobj.exe goto stop
if not exist dumpobj.exe goto stop
copy toobj.exe ..\bin\toobj.exe
copy dumpobj.exe ..\bin\dumpobj.exe
cd ..

echo Building see.exe

cd see
call make.bat
if not exist s.exe goto stop
copy s.exe ..\bin\see.exe
cd ..

echo Building tools

cd tools
call maketool.bat
copy *.exe ..\bin
copy *.s ..\bin
cd ..

echo Building more tools

cd src
c88 cb
bind cb
if not exist cb.exe goto stop
copy cb.exe ..\bin\cbcheck.exe
c88 dump
bind dump
if not exist dump.exe goto stop
copy dump.exe ..\bin\dump.exe
cd ..

echo Building libraries

cd lib_31
call make_lib.bat

echo Building big libraries

call makeblib.bat

copy bmylib.s ..\bin\bcstdio.s
copy bmylib7.s ..\bin\bcstdio7.s
copy mylib.s ..\bin\cstdio.s
copy mylib7.s ..\bin\cstdio7.s
cd ..

echo Copying include files

cd include
copy *.* ..\bin
cd ..

echo Building Optimizer

cd o
call make_o88.bat
if not exist my_cc.exe goto stop
copy my_cc.exe ..\bin\cc.exe
if not exist my_o88.exe goto stop
copy my_o88.exe ..\bin\o88.exe
copy released\macros.h ..\bin
cd ..

echo Build complete.
echo All files should now reside in bin directory.

goto ok

:stop

cd ..

:ok
