set O88.1=+aefjmpst
md obj

cc src\cc -oobj\cc -px -nOLD_ASM88 /sinc\cc.stk
if errorlevel 2 goto exit

cc src\gather -oobj\gather -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\o88 -oobj\o88 -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\parse -oobj\parse -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\utils -oobj\utils -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\8088-pt1 -oobj\8088-pt1 -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\8088-pt2 -oobj\8088-pt2 -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\8088-pt3 -oobj\8088-pt3 -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\80188 -oobj\80188 -px -nOLD_ASM88 /sinc\o88.stk
if errorlevel 2 goto exit

cc src\8087 -oobj\8087 -px -nPRODUCTION /sinc\o88.stk
if errorlevel 2 goto exit

cc src\labels -oobj\labels -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\asmlib -oobj\asmlib -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\allocate -oobj\allocate -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\squeeze -oobj\squeeze -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\jumps -oobj\jumps -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\folding -oobj\folding -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\buffers -oobj\buffers -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\cleanup -oobj\cleanup -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\linklist -oobj\linklist -px /sinc\o88.stk
if errorlevel 2 goto exit

cc src\data -oobj\data -px /sinc\o88.stk
if errorlevel 2 goto exit


copy ..\see\exec.ooo obj\exec.o
bind obj\exec obj\cc -s1000 -omy_cc -pcc.map -_

bind -ffiles -omy_o88 -po88.map -_

:exit
