@echo off
REM
REM Windows Batch file for launching ZCC to compile ZX TAP and launch in FUSE emulator.

set FILE=ttt
set EMUPATH="C:\Program Files (x86)\Fuse\fuse.exe"

echo Compiling %FILE%.c... 
zcc +zx -O3 -zorg=36864 %FILE%.c -lm -v -lndos -o %FILE% 
z88dk-appmake +zx -b %FILE% --org 36864 --screen zxscr.scr -o %FILE%.tap
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