# A.I. Tic Tac Toe - a simple Tic Tac Toe game

Copyright 2005 Marcello Zaniboni

https://www.marcellozaniboni.net/zxaittt/index.html

SAM Coupé conversion plus retatining support for compiling under ZX Spectrum

# Sam Coupé build

zcc +sam -O3 ttt.c -lm -v -lndos -o ttt.bin -pragma-define:CRT_STACK_SIZE=2048 -pragma-define:CLIB_DEFAULT_SCREEN_MODE=-1

Note: z88dk-appmake does not yet support the SAM target, you will need to build your own disk image with:
 * SAMDOS2
 * auto.bas - An Auto BASIC file of 10 MODE 3: CLS #: LOAD "ttt.bin" CODE: LOAD "samscr.scr" SCREEN$: PAUSE 50 : CALL 32768
 * samscr.scr
 * ttt.bin

or use cc-sam.bat

# ZX Spectrum build

zcc +zx -O3 -zorg=36864 ttt.c -lm -v -lndos -o ttt.bin 
z88dk-appmake +zx -b ttt.bin --org 36864 --screen zxscr.scr -o ttt.tap

or use cc-zx.bat
