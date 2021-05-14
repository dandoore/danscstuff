Auto 1
  10 CLEAR 36863
  20 BORDER 0: PAPER 0: INK 7: CLS
  30 PRINT AT 5,6;"+------------------+"
  40 PRINT AT 6,6;"| A.I. TIC TAC TOE |"
  50 PRINT AT 7,6;"+------------------+"
  60 PRINT AT 9,1;"(c) 2005 by Marcello Zaniboni"
  70 PRINT AT 15,11;"Graphics by"
  80 PRINT AT 16,3;"Luciano ""Lucky"" Costarelli"
  90 PRINT AT 19,3;"...loading, please wait..."
 115 INK 0
 120 LOAD ""CODE
 125 CLS : INK 4: PRINT AT 5,5;"please wait"
 130 INK 0
 140 LOAD ""SCREEN$
 180 BEEP .3,2: PAUSE 40
 190 RANDOMIZE USR 36864
 200 PAUSE 500: CLS
