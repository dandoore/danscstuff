@echo off
zcc +zx -O3 -zorg=36864 -create-app -vn -lndos -lm -o ttt ttt.c
del zcc_opt.def ttt
pause
