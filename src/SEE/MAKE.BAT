c88 see -px
if errorlevel 1 goto oops

c88 more -px
if errorlevel 1 goto oops

c88 wrap -px
if errorlevel 1 goto oops

c88 screen -px
if errorlevel 1 goto oops

c88 buffer -px
if errorlevel 1 goto oops

c88 block -px
if errorlevel 1 goto oops

c88 extmem -px
if errorlevel 1 goto oops

c88 macro -px
if errorlevel 1 goto oops

c88 view -px
if errorlevel 1 goto oops

c88 dosint -px
if errorlevel 1 goto oops

asm88 pcfast
if errorlevel 1 goto oops

asm88 move
if errorlevel 1 goto oops

bind see more screen wrap buffer view block extmem macro dosint move exec pcfast -os.exe

:oops
