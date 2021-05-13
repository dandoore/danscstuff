@echo off
REM
REM Windows Batch file for launching ZCC to compile SAM Coupé executables and lauch them in SimCoupe via
REM an MGT disk image.
REM
REM https://github.com/dandoore/mgtman

set FILE=ttt
set OUTPUT=ttt.cde
set EMUPATH="C:\Program Files (x86)\SimCoupe\simcoupe.exe"

echo Compiling %FILE%.c... 
zcc +sam -O3 %FILE%.c -lm -v -lndos -o %FILE% -pragma-define:CRT_STACK_SIZE=2048  -pragma-define:CLIB_DEFAULT_SCREEN_MODE=-1
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing old %FILE%.mgt file if needed...
IF EXIST %FILE%.mgt del %FILE%.mgt 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Copy skelton.mgt to %FILE%.mgt file...
COPY skeleton.mgt %FILE%.mgt 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Renaming BIN to %OUTPUT%...
rename %FILE% %OUTPUT%
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Copying auto.bin to %FILE%.mgt...
mgtman -w %FILE%.mgt %OUTPUT% 32768 
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing binary %OUTPUT%...
del %OUTPUT% 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Launching %FILE%.mgt with SimCoupe...
call %EMUPATH% %FILE%.mgt -autoload 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

GOTO END

:FAIL
Echo  ^^ %FILE% failed, press a key.
pause 1> nul 2> nul
:END