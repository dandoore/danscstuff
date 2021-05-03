@echo off
REM
REM Windows Batch file for launching ZCC to compile SAM Coupé executables and lauch them in SimCoupe via
REM an MGT disk image.

set FILE=danscdemo
set EMUPATH="C:\Program Files (x86)\SimCoupe\simcoupe.exe"

echo Compiling %FILE%.c... 
zcc +sam %FILE%.c cdemo.asm -lm -v -lndos -o %FILE% -create-app -pragma-redirect:CRT_FONT=_font -pragma-define:CRT_STACK_SIZE=2048
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing binary %FILE%...
del %FILE% 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Launching %FILE%.mgt with SimCoupe...
call %EMUPATH% %FILE%.mgt -autoload 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

GOTO END

:FAIL
Echo  ^^ %FILE% failed, press a key.
pause 1> nul 2> nul
:END