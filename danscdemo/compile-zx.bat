@echo off
REM
REM Windows Batch file for launching ZCC to compile ZX executables and lauch them in FUSE

set FILE=danscdemo

set EMUPATH="C:\Program Files (x86)\Fuse\fuse.exe"

echo Compiling %FILE%.c... 
zcc +zx -v %FILE%.c zx_cdemo.asm -lndos -o %FILE% -create-app -pragma-define:CRT_STACK_SIZE=4096 -pragma-redirect=CRT_FONT=_font_8x8_clairsys_bold

IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing binary %FILE%...
del %FILE% 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Launching %FILE%.tap with FUSE...
call %EMUPATH% %FILE%.tap
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

GOTO END

:FAIL
Echo  ^^ %FILE% failed, press a key.
pause 1> nul 2> nul
:END