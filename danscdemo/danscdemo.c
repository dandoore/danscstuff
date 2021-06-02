// Dan's C Demo by Dan Dooré (C)1996 BANZAI PRODUCTIONS
//
// Centre Starfield, Trailblazer, Life and Parallax Starfield
//
// The first two routines I wrote in BASIC but were so slow as
// to make them totally useless, but a quick bash in C soon
// sorted that out.
//
// Centre Starfield was from an article in Computer Shopper and
// Trailblazer is comparable to Andy Wright's 'Lines' demo on
// the SamDOS 2 disc but soooo much faster!
//
// Life was thieved from a PC C Compiler disc (PD, of course)
// and is John Conway's life algorithm.
//
// Parallax starfield was something I threw together and isn't
// very efficient, but it works.
//
// Both starfields use SAM palette pos 1-3 for the stars and operate
// on a background of pos 0.  Stars will not trash objects in
// the foreground if they don't contain any of the pot colours.
// (Not supported in Z88DK yet)
//
// 2021 remix for Z88DK (https://www.z88dk.org/
//
// SAM Coupé (Native)
// ==================
//
// SAM SAA1099 Etracker by Pyramex - Brutal 8 E (Altern-8)
//
// The ASM contains the includes for the SAM font and Etracker music, use -pragma-redirect:CRT_FONT=_font to enable custom font
// Increase the stack size for the arrays with -pragma-define:CRT_STACK_SIZE=2048
//
// zcc +sam danscdemo.c cdemo.asm -lm -lndos -create-app -pragma-redirect:CRT_FONT=_font -pragma-define:CRT_STACK_SIZE=2048 
//
// ZX Spectrum
// ===========
//
// ZX AY Soundtracker by Yerzmyey - Real Life Super Hero
// 
// The ASM contains the includes for the ProTracker 3 music, use -pragma-redirect:CRT_FONT=_font_8x8_clairsys_bold to enable different font
// Lower ORG address and increase the stack size for the arrays with -pragma-define:CRT_STACK_SIZE=2048
//
// zcc +zx -O3 -v -zorg=30000 danscdemo.c zx_cdemo.asm -lndos -create-app -pragma-define:CRT_STACK_SIZE=2048 -pragma-redirect=CRT_FONT=_font_8x8_clairsys_bold

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <intrinsic.h>
#include <interrupt.h>

#if defined(SAM)
#include <arch/sam.h>
#include <psg/etracker.h>
#include <sys/ioctl.h>
#endif

#if defined(SPECTRUM)
#include <spectrum.h>
#include <psg/vt2.h>
#endif

// Prototypes

#if defined(SAM)
void fade(int x, int * y);
void fadedown(int x, int * y);
void fadeup(int x, int * y);
#endif
void playmusic();
void setup_int();
int rnd(int x);
int keypress();
void cleary();
void intro();
void startstars();
void preprinty(int x, int y, char * z);
void printy(int x, int y, char * z);
void blank(int x, char * y);
void smove();
void trailblazer();
void life();
void setuplife();
void cyclelife();
void addd(int x, int y);
void outputlife();
void outro();
void pstartup();
void psmove();
void outpreprinty(int x, char * y);
void outprinty(int x, char * y);

// Defines

#define STARS 30 // Number of stars in centre starfield in Intro
#define SPREAD 30 // Birth range spread of new stars in centre starfield

#define TEXT 7 // Default foreground colour
#define BACK 0 // Default background colour
#define SAMTEXT 15 //  Default foreground colour SAM
#define SAMINVIS 4 // Invisible text colour for SAM

#define TRAILS 40 // Number of 'trails' for Trailblazer
#define OFFSET 10 // Trailblazer offset to avoid overprinting title
#define MINX 0 // Trailblazer origin x
#define MINY 0 // Trailblazer origin y
#define MAXX 255 // Trailblazer max x
#define MAXY 192 // Trailblazer max y

#define ROWS 21 // Life size y rows
#define COLS 32 // Life size x columns
#define LIFESEED 46256 // Because everyone wants a happy life. This seed is known good for 350 generations

#define PSTARS 20 // Number of parallax stars in Outro

// Globals

char mess1[] = "Welcome to Dan's C Demo";
char mess2[] = "Written in Z88DK";
char mess3[] = "Press a key to move on...";

char mess4[] = "Originally written in SamC";
char mess5[] = "For FRED magazine 1996";
char mess6[] = "Z88DK version in 2021";
char mess7[] = "by Dan Doore";
#if defined(SAM)
char mess8[] = "Music by Pyramex";
#endif
#if defined(SPECTRUM)
char mess8[] = "Music by Yerzmyey";
#endif
char mess9[] = "www.z88dk.org";

#if defined(SAM)
// Palette defintions

int grey[7] = {
  119,
  120,
  112,
  15,
  7,
  8,
  0
};
int red[7] = {
  38,
  39,
  34,
  40,
  32,
  2,
  0
};
int blue[7] = {
  31,
  28,
  24,
  17,
  16,
  1,
  0
};
int copper[7] = {
  68,
  72,
  64,
  66,
  12,
  4,
  0
};

char defaultpal[16] = {
  0,
  7,
  112,
  127,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  68,
  31,
  38,
  119,
  127
};
char fullcopper[16] = {
  0,
  4,
  12,
  66,
  64,
  72,
  68,
  76,
  68,
  72,
  64,
  66,
  12,
  4,
  0,
  127
};
#endif

int moveon, count, speed, cx, cy;

int starx[STARS], stary[STARS], starskip[STARS];
int pstarx[PSTARS], pstary[PSTARS], pstars[PSTARS], pstarskip[PSTARS];

char world[ROWS][COLS];
int popul, gen, crow, ccol;

// Main!

void main() {
  #if defined(SAM)
  // Screen setup
  int mode = 4;
  console_ioctl(IOCTL_GENCON_SET_MODE, & mode);
  // Music setup
  saa_etracker_init( & mysong);
  setup_int();
  #endif

  #if defined(SPECTRUM)
  // Screen setup
  printf("%c%c\n", 1, 32); // 32 column mode
  textcolor(TEXT);
  textbackground(BACK);
  zx_border(BACK);
  clrscr();

  // Clear out anything in the keyboard first
  while (keypress());

  // Music setup
  ay_vt2_init( & mysong);
  ay_vt2_start();
  setup_int();
  #endif

  while (1) {
    #if defined(SAM)
    sam_load_palette(defaultpal);
    #endif
    intro();
    cleary();
    trailblazer();
    cleary();
    life();
    cleary();
    outro();
    cleary();
  }
  exit(0);
}

// ZX VortexTracker tune

#if defined(SPECTRUM)
extern vt2_song mysong;

// Setup interrupt tracker player

void setup_int() {

  zx_im2_init(0xd300, 0xd4);
  add_raster_int(0x38);
  add_raster_int(playmusic);
}

// Interrupt tracker player routine

void playmusic(void) {
  M_PRESERVE_MAIN;
  M_PRESERVE_INDEX;
  ay_vt2_play();
  M_RESTORE_INDEX;
  M_RESTORE_MAIN;
}
#endif

// SAM Etracker tune

#if defined(SAM)
extern char mysong;

// Setup interrupt tracker player

void setup_int() {
  add_raster_int(playmusic);
}
// Interrupt tracker player routine

void playmusic(void) {
  M_PRESERVE_MAIN;
  saa_etracker_play();
  M_RESTORE_MAIN;
}
#endif

// RND Function for integers 0-32768

int rnd(int range) {
  return abs(abs(rand()) / (32768 / range));
}

// Detect Keypress Routine

int keypress() {
  char dummyval;
  if (kbhit()) {
    dummyval = getch();
    moveon = 1;
    return (1);
  } else {
    return (0);
  }
}

// Clear the screen in a slightly more interesting way
// This is really slow at the moment :(

void cleary() {
  #if defined(SAM)
  int x;
  textcolor(BACK);
  for (x = MINX; x <= MAXX; x += 2) {
    draw(x, MINY, x, MAXY);
    draw(MAXX - x, MINY, MAXX - x, MAXY);
  }
  #endif

  #if defined(SPECTRUM)
  int x;
  for (x = MINX; x <= MAXX; x += 2) {
    undraw(x, (MINY + 1), x, (MAXY - 1));
    undraw(MAXX - x, (MINY + 1), MAXX - x, (MAXY - 1));
  }
  textcolor(TEXT);
  textbackground(BACK);
  zx_border(BACK);
  #endif

  #if defined(SAM)
  sam_load_palette(defaultpal);
  #endif

  clrscr();
}

// Intro section

void intro() {
  moveon = 0;
  #if defined(SAM)
  preprinty(2, 11, mess1);
  preprinty(10, 13, mess2);
  preprinty(18, 12, mess3);
  #endif
  startstars();

  while (!keypress()) {
    printy(2, 11, mess1);
    if (moveon) break;
    printy(10, 13, mess2);
    if (moveon) break;
    printy(18, 12, mess3);
    if (moveon) break;
    #if defined(SAM)
    fade(11, copper);
    #endif
    blank(2, mess1);
    if (moveon) break;
    #if defined(SAM)
    fade(13, red);
    #endif
    blank(10, mess2);
    if (moveon) break;
    #if defined(SAM)
    fade(12, blue);
    #endif
    blank(18, mess3);
    if (moveon) break;
    #if defined(SAM)
    sam_load_palette(defaultpal);
    #endif
  }
  moveon = 0;
}

// Routine for pre printing intro texts
//
// Kludge for lack of POINT(x,y) being able to return pen value rather than boolean - pre print message in black text :)

void preprinty(int lineno, int colour, char * message) {
  gotoxy((32 - strlen(message)) / 2, lineno);
  textcolor(SAMINVIS);
  printf("%s", message);
}

// Routine for printing intro texts

void printy(int lineno, int colour, char * message) {
  int x;

  gotoxy((32 - strlen(message)) / 2, lineno);
  for (x = 0; x < strlen(message); x++) {
    #if defined(SAM)
    textcolor(colour);
    #endif
    #if defined(SPECTRUM)
    textcolor(colour - 10);
    #endif
    printf("%c", message[x]);
    if (keypress()) {
      moveon = 1;
      break;
    }
    smove();
  }
}

// Fade colour pot using a colour range
#if defined(SAM)
void fade(int colour, int * colset) {
  int x;

  for (x = 0; x < 7; x++) {
    sam_set_palette(colour, colset[x]);
    smove();
    if (keypress()) break;
  }
}
#endif

// Blank a text line without distrubing screen too much

void blank(int lineno, char * message) {
  gotoxy((32 - strlen(message)) / 2, lineno);

  #if defined(SAM)
  textcolor(SAMINVIS); // Since POINT(X,Y) can't distinguish colours yet make the text invisible but still there so that stars will not overprint it
  printf("%s", message);
  smove();
  #endif
  #if defined(SPECTRUM) // There's no fade on ZX so make it a bit more animated
  int x;
  for (x = 0; x <= strlen(message); x++) {
    textcolor(BACK); // Make the text invisible but still there so that stars will not overprint it
    printf(" ");
    if (keypress()) break;
    smove();
  }
  #endif
}

// Initalise and print starfield

void startstars() {
  speed = 4;
  cx = 128;
  cy = 96;

  #if defined(SAM)
  textcolor(1);
  #endif
  #if defined(SPECTRUM)
  textcolor(TEXT);
  #endif
  for (count = 0; count <= (STARS - 1); count++) {
    starx[count] = (rnd(SPREAD) + (cx - (SPREAD / 2)));
    stary[count] = (rnd(SPREAD) + (cy - (SPREAD / 2)));
    if (point(starx[count], stary[count]) == NULL) // zx_pxy2aaddr(px,py) will return the attribute address corresponding to the given (x,y) pixel coordinate
    {
      starskip[count] = 0;
      plot(starx[count], stary[count]);
    } else {
      starskip[count] = 1;
    }
  }
}

// Move the starfield once

void smove() {
  int dx, dy, newx, newy, birth;

  //intrinsic_halt();  // Wait for start of next frame

  for (count = 0; count <= (STARS - 1); count++) {

    birth = 0;
    dx = (starx[count] - cx) / speed;
    dy = (stary[count] - cy) / speed;

    if ((dx == 0) && (dy == 0)) {
      dx = 1;
      dy = 2;
    }
    newx = (starx[count] + dx);
    newy = (stary[count] + dy);

    if ((newx <= 0 || newx >= 254) || (newy <= 0 || newy >= 190)) {
      newx = (rnd(SPREAD) + (cx - (SPREAD / 2)));
      newy = (rnd(SPREAD) + (cy - (SPREAD / 2)));
      birth = 1;
    }
    // Remove old star position - should check that POINT(starx[count],stary[count])=0 i.e not printed text but this is not supported yet

    if (starskip[count] == 0) {
      #if defined(SAM)
      textcolor(0);
      plot(starx[count], stary[count]);
      #endif
      #if defined(SPECTRUM)
      // ADD: UNPLOT ony if attribute of pixel is backgorund
      unplot(starx[count], stary[count]);
      #endif
    }

    // Plot new star position

    if (point(newx, newy) == NULL) // ADD: PLOT ony if ZX attribute of pixel is background
    {
      starskip[count] = 0;
      if (((abs(dx)) + (abs(dy))) < 10) {
        #if defined(SAM)
        textcolor(2);
        #endif
        #if defined(SPECTRUM)
        textcolor(TEXT);
        #endif
        if (((abs(dx)) + (abs(dy))) < 3)
          #if defined(SAM)
        textcolor(1);
        #endif
        #if defined(SPECTRUM)
        textcolor(TEXT);
        #endif
      } else
        #if defined(SAM)
      textcolor(3);
      #endif
      #if defined(SPECTRUM)
      textcolor(TEXT);
      #endif

      if (birth)
        #if defined(SAM)
      textcolor(1);
      #endif
      #if defined(SPECTRUM)
      textcolor(TEXT);
      #endif

      plot(newx, newy);
    } else {
      starskip[count] = 1;
    }
    starx[count] = newx;
    stary[count] = newy;
  }
}

// TrailBlazer

// Trailblazer uses wrap-around queues for maximum speed
// (no need to shift data in the queues therefore the number of
// trails doesn't cause a time penalty)

void trailblazer() {
  int line1x[TRAILS], line1y[TRAILS];
  int line2x[TRAILS], line2y[TRAILS];

  int dx1, dy1, dx2, dy2, start, end, last;
  int new1x, new1y, new2x, new2y;
  unsigned palcount;

  // Set random start position for trailblazer

  line1x[0] = rnd(MAXX - MINX);
  line1y[0] = rnd(MAXY - (MINY + OFFSET));
  line2x[0] = rnd(MAXX - MINX);
  line2y[0] = rnd(MAXY - (MINY + OFFSET));

  // Set all other trails to this value

  for (count = 1; count <= (TRAILS - 1); count++) {
    line1x[count] = line1x[0];
    line1y[count] = line1y[0];
    line2x[count] = line2x[0];
    line2y[count] = line2y[0];
  }

  // Initial movement values (signed value)

  dx1 = 2;
  dy1 = 3;
  dx2 = -3;
  dy2 = -2;

  // Queue variables

  start = 0;
  end = (TRAILS - 1);

  // Title
  #if defined(SAM)
  sam_load_palette(fullcopper);
  #endif
  textcolor(SAMTEXT);
  textbackground(BACK);
  gotoxy(0, 0);
  printf("TrailBlazer - Press key for next");

  palcount = 0;
  moveon = 0;

  // Main loop

  while (!keypress() & !moveon) {
    // Resolve pointer to the last line values and wrap if need be

    last = (start + 1);
    if (last == TRAILS) last = 0;

    // Set 'New' values with co-ords +/- movement values from last's

    new1x = line1x[last] + dx1;
    new1y = line1y[last] + dy1;
    new2x = line2x[last] + dx2;
    new2y = line2y[last] + dy2;

    // if 'New' co-ords over-range then invert the movement values

    if (new1x <= MINX) dx1 -= (dx1 * 2);
    if (new2x <= MINX) dx2 -= (dx2 * 2);
    if (new1y <= (MINY + OFFSET)) dy1 -= (dy1 * 2);
    if (new2y <= (MINY + OFFSET)) dy2 -= (dy2 * 2);
    if (new1x >= MAXX) dx1 -= (dx1 * 2);
    if (new2x >= MAXX) dx2 -= (dx2 * 2);
    if (new1y >= MAXY) dy1 -= (dy1 * 2);
    if (new2y >= MAXY) dy2 -= (dy2 * 2);

    // Now safe to set front of trail to last + movement values

    line1x[start] = line1x[last] + dx1;
    line1y[start] = line1y[last] + dy1;
    line2x[start] = line2x[last] + dx2;
    line2y[start] = line2y[last] + dy2;

    // Draw trail front

    #if defined(SAM)
    textcolor((palcount % 12) + 1);
    #endif
    #if defined(SPECTRUM)
    textcolor(TEXT);
    #endif
    draw(line1x[start], line1y[start], line2x[start], line2y[start]);

    // Blank out last line of trail

    #if defined(SAM)
    textcolor(BACK);
    draw(line1x[end], line1y[end], line2x[end], line2y[end]);
    #endif
    #if defined(SPECTRUM)
    undraw(line1x[end], line1y[end], line2x[end], line2y[end]);
    #endif
    if (palcount == 60000) palcount = 0;
    palcount++;

    // Shift Queue pointers

    end--;
    start--;
    if (end < 0) end = (TRAILS - 1);
    if (start < 0) start = (TRAILS - 1);
  }
  moveon = 0;
}

// Life, it's the name of the game

void life() {
  #if defined(SAM)
  textcolor(SAMTEXT);
  #endif
  #if defined(SPECTRUM)
  textcolor(TEXT);
  #endif
  setuplife();
  moveon = 0;

  while (!keypress() && popul) {
    gen++;
    gotoxy(0, 23);
    printf("Generation:%4d Population:%3d", gen, popul);
    cyclelife();
    if (moveon || keypress()) break;
    outputlife();
    if (moveon || keypress()) break;
  }
  moveon = 0;
}

// Setup Life function

void setuplife() {
  int rnumber, i, row, col;

  srand(LIFESEED);

  gen = 0;
  rnumber = rnd(60) + 60;
  popul = rnumber;

  gotoxy(0, 22);
  printf("Life - Press key for next part\n");
  printf("Generation:%4d Population:%3d", gen, popul);

  for (i = 0; i < rnumber; i++) {
    row = rnd(ROWS);
    col = rnd(COLS);
    world[row][col] = 'X'; // putchar in a cell
    gotoxy(col, row);
    putchar('O');
  }
}

// Update screen and world

void outputlife() {
  int row, col;
  char cell;

  popul = 0;
  for (row = 0; row < ROWS; row++) {
    for (col = 0; col < COLS; col++) {
      cell = world[row][col];
      if (cell && (cell == 3 || cell == 'X' + 2 || cell == 'X' + 3)) {
        popul++;

        if (cell < 'X') {
          gotoxy(col, row);
          putchar('O');
        }
        cell = 'X';
      } else {
        if (cell >= 'X') {
          gotoxy(col, row);
          putchar(' ');
        }
        cell = 0;
      }
      world[row][col] = cell;
    }
  }
}

// Generate next life cycle

void cyclelife() {
  int row, col;

  // Take care of left and right column first

  for (row = 0; row < ROWS; row++) {
    if (world[row][0] >= 'X') addd(row, 0);
    if (world[row][COLS - 1] >= 'X') addd(row, COLS - 1);
  }

  // Take care of top and bottom line

  for (col = 1; col < COLS - 1; col++) {
    if (world[0][col] >= 'X') addd(0, col);
    if (world[ROWS - 1][col] >= 'X') addd(ROWS - 1, col);
    if (keypress()) break;
  }

  // Fill in the box, ignoring border conditions

  for (row = 1; row < ROWS - 1; row++) {
    for (col = 1; col < COLS - 1; col++) {
      if (world[row][col] >= 'X') {
        world[row - 1][col - 1]++;
        world[row - 1][col]++;
        world[row - 1][col + 1]++;
        world[row][col - 1]++;
        world[row][col + 1]++;
        world[row + 1][col - 1]++;
        world[row + 1][col]++;
        world[row + 1][col + 1]++;
        if (keypress()) break;
      }
    }
  }
}

// Addddddd

void addd(int row, int col) {
  int rrow, ccol, rr, cc;

  for (rr = row - 1; rr <= row + 1; rr++) {
    for (cc = col - 1; cc <= col + 1; cc++) {
      rrow = rr != -1 ? rr : ROWS - 1;
      ccol = cc != -1 ? cc : COLS - 1;
      if (rrow >= ROWS) rrow = 0;
      if (ccol >= COLS) ccol = 0;
      world[rrow][ccol]++;
    }
  }
  world[row][col]--;
}

// Outro section

void outro() {
  moveon = 0;
  #if defined(SAM)
  outpreprinty(3, mess4);
  outpreprinty(6, mess5);
  outpreprinty(9, mess6);
  outpreprinty(12, mess7);
  outpreprinty(15, mess8);
  outpreprinty(18, mess9);
  #endif
  pstartup();

  while (!keypress()) {
    outprinty(3, mess4);
    if (moveon) break;
    outprinty(6, mess5);
    if (moveon) break;
    outprinty(9, mess6);
    if (moveon) break;
    outprinty(12, mess7);
    if (moveon) break;
    outprinty(15, mess8);
    if (moveon) break;
    outprinty(18, mess9);

    while (!moveon) {
      #if defined(SAM)
      fadedown(11, copper);
      if (keypress()) break;
      fadedown(12, red);
      if (keypress()) break;
      fadedown(13, blue);
      if (keypress()) break;
      fadedown(14, grey);
      if (keypress()) break;
      fadeup(11, copper);
      if (keypress()) break;
      fadeup(12, red);
      if (keypress()) break;
      fadeup(13, blue);
      if (keypress()) break;
      fadeup(14, grey);
      #endif
      #if defined(SPECTRUM)
      if (keypress()) break;
      psmove();
      #endif
    }
  }
  moveon = 0;
}

// Startup routine for parallax stars

void pstartup() {
  textcolor(1);
  for (count = 0; count <= (PSTARS - 1); count++) {
    pstarx[count] = rnd(255);
    pstary[count] = rnd(191);
    pstars[count] = (count % 3) + 1;

    if (point(pstarx[count], pstary[count]) == NULL) {
      pstarskip[count] = 0;
      #if defined(SAM)
      textcolor(pstars[count]);
      #endif
      #if defined(SPECTRUM)
      textcolor(TEXT);
      #endif
      plot(pstarx[count], pstary[count]);
    } else {
      pstarskip[count] = 1;
    }
  }
}

// Move stars

void psmove() {
  //intrinsic_halt();	// Wait for start of next frame
  for (count = 0; count <= (PSTARS - 1); count++) {
    if (pstarskip[count] == 0) // Blank old pixel is not ignored
    {
      #if defined(SAM)
      textcolor(BACK);
      plot(pstarx[count], pstary[count]);
      #endif
      #if defined(SPECTRUM)
      unplot(pstarx[count], pstary[count]);
      #endif
    }

    pstarx[count] += pstars[count];
    if (pstarx[count] > 255) pstarx[count] = 0;

    if (point(pstarx[count], pstary[count]) == NULL) {
      pstarskip[count] = 0;
      #if defined(SAM)
      textcolor(pstars[count]);
      #endif
      #if defined(SPECTRUM)
      textcolor(TEXT);
      #endif
      plot(pstarx[count], pstary[count]);
    } else {
      pstarskip[count] = 1;
    }
  }
}

// Outro text pre print in invisible text so it's avoided by starfield

void outpreprinty(int lineno, char * message) {
  #if defined(SAM)
  gotoxy((32 - strlen(message)) / 2, lineno);
  textcolor(SAMINVIS);
  printf("%s", message);
  #endif
}

// Routine for printing outro texts

void outprinty(int lineno, char * message) {
  int x;

  gotoxy((32 - strlen(message)) / 2, lineno);

  for (x = 0; x < strlen(message); x++) {
    #if defined(SAM)
    textcolor((x % 4) + 11);
    #endif
    #if defined(SPECTRUM)
    textcolor((lineno % 4) + 11);
    #endif
    printf("%c", message[x]);
    if (keypress()) break;
    psmove();
  }
}

// Outro Fade down
#if defined(SAM)
void fadedown(int colour, int * colset) {

  int x;
  for (x = 0; x < 5; x++) {
    sam_set_palette(colour, colset[x]);
    psmove();
    if (keypress()) break;
  }
}

// Outro Fade up

void fadeup(int colour, int * colset) {
  int x;
  for (x = 4; x > 0; x--) {
    sam_set_palette(colour, colset[x]);
    psmove();
    if (keypress()) break;
  }

}
#endif
