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
// Both starfields use palette pos 1-3 for the stars and operate
// on a background of pos 0.  Stars will not trash objects in
// the foreground if they don't contain any of the pot colours.
//
// 2021 remix for Z88DK
//
// SAM SAA1099 Etracker by Pyramex
// ZX AY Soundtracker by Yerzmyey
//
// Compilation under Z88DK (https://www.z88dk.org/
//
// SAM Coupé (Native)
// ==================
//
// The ASM contains the includes for the SAM font and Etracker music, use -pragma-redirect:CRT_FONT=_font to enable font
// Increase the stack size for the arrays for Trailblazer and Life with -pragma-define:CRT_STACK_SIZE=2048
//
// zcc +sam danscdemo.c cdemo.asm -lm -lndos -create-app -pragma-redirect:CRT_FONT=_font -pragma-define:CRT_STACK_SIZE=2048 
//
// ZX Spectrum (Experimental)
// ==========================
//
// zcc +zx danscdemo.c -lm -lndos -create-app -pragma-define:CRT_STACK_SIZE=2048 -pragma-redirect=CRT_FONT=_font_8x8_clairsys_bold

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <intrinsic.h>

#if defined(SAM)
#include <interrupt.h>
#include <psg/etracker.h>
#include <sys/ioctl.h>
#include <arch/sam.h>
#endif

#if defined(SPECTRUM)
#include <spectrum.h>
#include <input.h>
#include <arch/z80.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <vt_sound.h>
#endif

// Pragmas

#if defined(SPECTRUM)
#pragma output CRT_ORG_CODE = 0x8184
#pragma output CLIB_MALLOC_HEAP_SIZE = 0
#endif

// Prototypes

#if defined(SAM)
void playmusic();
void setup_int();
#endif
#if defined(SPECTRUM)
static void init_isr();
#endif

void fade(int x, int *y);
void fadedown(int x, int *y);
void fadeup(int x, int *y);
int rnd(int x);
int keypress();
void cleary();
void intro();
void startstars();
void printy(int x, int y, char *z);
void blank(int x, char *y);
void smove();
void trailblazer();
void life();
void setuplife();
void cyclelife();
void addd(int x,int y);
void outputlife();
void outro();
void pstartup();
void psmove();
void outprinty(int x, char *y);


// Defines

#define STARS  30   // Number of stars in centre starfield in Intro
#define SPREAD 30   // Birth range spread of new stars in centre starfield

#define TEXT 7		// Default foreground colour
#define BACK 0		// Default background colour

#define TRAILS 40   // Number of 'trails' for Trailblazer
#define MINX 0		// Trailblazer
#define	MINY 0		// Trailblazer
#define OFFSET 10	// Trailblazer
#define MAXX 255	// Trailblazer
#define	MAXY 192	// Trailblazer

#define ROWS  21    // Life size y 
#define COLS  32    // Life size x 
#define LIFESEED	46256  // Because everyone wants a happy life. This seed is known good for 350 generations

#define PSTARS 20   // Number of parallax stars in Outro

	
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

int grey[7]   = { 119,120,112,15,7,8,0 };
int red[7]    = { 38,39,34,40,32,2,0 };
int blue[7]   = { 31,28,24,17,16,1,0 };
int copper[7] = { 68,72,64,66,12,4,0 };

char defaultpal[16] = { 0,7,112,127,0,0,0,0,0,0,0,68,31,38,119,127 };
char fullcopper[16] = { 0,4,12,66,64,72,68,76,68,72,64,66,12,4,0,127 };

int moveon,count,speed,cx,cy;

int starx[STARS],stary[STARS];
int pstarx[PSTARS],pstary[PSTARS],pstars[PSTARS];

char world[ROWS][COLS];
int popul,gen,crow,ccol;

// Main!

void main()
{
#if defined(SAM)
int	mode=4;

console_ioctl(IOCTL_GENCON_SET_MODE,&mode);
saa_etracker_init(&mysong);
setup_int();
#endif

#if defined(SPECTRUM)
printf("%c%c\n",1,32);		// 32 column mode
textcolor(TEXT);
textbackground(BACK);
zx_border(BACK);
clrscr();
vt_init(music_module);
init_isr();

#endif

while (1)
    {
	#if defined(SAM)
	sam_load_palette(defaultpal);
	#endif
	intro();
	cleary();
	//trailblazer();
	//cleary();
	//life();
	//cleary();
	//outro();
	//cleary();
    }
exit(0);
}

#if defined(SPECTRUM)

extern uint8_t music_module[];			// ProTracker3 tune

static void init_isr(void)
{
    // Set up IM2 interrupt service routine:
    // Put Z80 in IM2 mode with a 257-byte interrupt vector table located
    // at 0x8000 (before CRT_ORG_CODE) filled with 0x81 bytes. Install the
    // vt_play_isr() interrupt service routine at the interrupt service routine
    // entry at address 0x8181.

    intrinsic_di();
    im2_init((void *) 0x8000);
    memset((void *) 0x8000, 0x81, 257);
    z80_bpoke(0x8181, 0xC3);
    z80_wpoke(0x8182, (uint16_t) vt_play_isr);
    intrinsic_ei();
}
#endif

#if defined(SAM)
extern char mysong;			// Etracker tune

// Setup interrupt tracker player

void setup_int()
{
add_raster_int(playmusic);
}
// Interrupt tracker player routine

void playmusic(void)
{
M_PRESERVE_MAIN;
saa_etracker_play();
M_RESTORE_MAIN;
}
#endif

// RND Function for integers 0-32768

int rnd(int range)
{
return abs(abs(rand())/(32768/range));
}

// Detect Keypress Routinr

int keypress()
{
char dummyval;
	if (kbhit())
		{
		dummyval=getch();
		moveon=1;
		return(1);
		}
	else
		{
		return(0);
		}
}

// Clear the screen in a slightly more interesting way
// This is really slow at the moment :(

void cleary()
{
int x;

textcolor(BACK);
for (x=MINX;x<= MAXX;x+= 2 )
	{
	draw(x,MINY,x,MAXY);
	draw(MAXX-x,MINY,MAXX-x,MAXY);
	}

#if defined(SAM)
sam_load_palette(defaultpal);
#endif
#if defined(SPECTRUM)
textcolor(TEXT);
textbackground(BACK);
#endif
clrscr();
}

// Intro section

void intro()
{
moveon=0;
startstars();
while (!keypress())
    {
    printy(2,11,mess1);
    if (moveon) break;
    printy(10,13,mess2);
    if (moveon) break;
    printy(18,12,mess3);
    if (moveon) break;
    fade(11,copper);
    blank(2,mess1);
    if (moveon) break;
    fade(13,red);
    blank(10,mess2);
    if (moveon) break;
    fade(12,blue);
    blank(18,mess3);
	if (moveon) break;
	#if defined(SAM)
    sam_load_palette(defaultpal);
	#endif
    }
moveon=0;
}

// Routine for printing intro texts

void printy(int lineno, int colour, char *message)
{
int x;

gotoxy((32-strlen(message))/2,lineno);
for (x=0;x< strlen(message);x++ )
    {
    #if defined(SAM)
	textcolor(colour);
	#endif
	#if defined(SPECTRUM)
	textcolor(colour-8);
	#endif
    printf("%c",message[x]);
    if (keypress()) break;
    smove();
    }
}

// Fade colour pot using a colour range

void fade(int colour, int *colset)
{
int x;

#if defined(SAM)
for (x=0;x<7;x++ )
    {
    sam_set_palette(colour,colset[x]);
    smove();
    if (keypress()) break;
    }
#endif

#if defined(SPECTRUM)
   smove();
#endif
}

// Blank a text line without distrubing screen too much

void blank(int lineno, char *message)
{
gotoxy((32-strlen(message))/2,lineno);

#if defined(SAM)
textcolor(BACK);
printf("%s",message);
smove();
#endif
#if defined(SPECTRUM)			// There's no fade on ZX so make it a bit more animated
int x;
for (x=0;x<=strlen(message);x++)
	{
	printf(" ");	
	if (keypress()) break;
	smove();
	}	
#endif
}

// Initalise and print starfield

void startstars()
{
speed = 4;
cx = 128;
cy = 96;

#if defined(SAM)
textcolor(1);
#endif
#if defined(SPECTRUM)
textcolor(TEXT);
#endif
for (count=0;count<= (STARS-1);count++ )
    {
    starx[count] = (rnd(SPREAD)+(cx-(SPREAD/2)));
    stary[count] = (rnd(SPREAD)+(cy-(SPREAD/2)));
    if (point(starx[count],stary[count]) == NULL)
        plot(starx[count],stary[count]);
    }
}

// Move the starfield once

void smove()
{
int dx,dy,newx,newy,birth;

//intrinsic_halt();  // Wait for start of next frame
for (count=0;count<= (STARS-1);count++ )
    {

    birth=0;
    dx = (starx[count] - cx)/speed;
    dy = (stary[count] - cy)/speed;

    if ((dx == 0) && (dy == 0))
        {
        dx = 1;
        dy = 2;
        }
    newx = (starx[count] + dx);
    newy = (stary[count] + dy);

   if ((newx <= 0 || newx >= 254) || (newy <= 0 || newy >= 190))
        {
        newx = (rnd(SPREAD)+(cx - (SPREAD/2)));
        newy = (rnd(SPREAD)+(cy - (SPREAD/2)));
        birth=1;
        }
   
    #if defined(SAM)
	if (point(starx[count],stary[count]) <= 3)
		{
		textcolor(0);
		plot(starx[count],stary[count]);
		}
	#endif
	#if defined(SPECTRUM)
	if (point(starx[count],stary[count]) == 1)
		{
		unplot(starx[count],stary[count]);
		}
    #endif
	

    if (point(newx,newy) == 0)
    {
    if (((abs (dx))+(abs (dy))) < 10)
        {
		#if defined(SAM)
		textcolor(2);
		#endif
		#if defined(SPECTRUM)
		textcolor(TEXT);
		#endif
        if (((abs (dx))+(abs (dy))) < 3)
           #if defined(SAM)
			textcolor(1);
			#endif
			#if defined(SPECTRUM)
			textcolor(TEXT);
			#endif
        }
    else
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
    plot(newx,newy);
    }
    starx[count] = newx;
    stary[count] = newy;
    }
}

// TrailBlazer

// Trailblazer uses wrap-around queues for maximum speed
// (no need to shift data in the queues therefore the number of
// trails doesn't cause a time penalty)

void trailblazer()
{
int line1x[TRAILS],line1y[TRAILS];
int line2x[TRAILS],line2y[TRAILS];

int dx1,dy1,dx2,dy2,start,end,last;
int new1x,new1y,new2x,new2y;
unsigned palcount;

// Set random start position for trailblazer

line1x[0]=rnd(MAXX-MINX);
line1y[0]=rnd(MAXY-(MINY+OFFSET));
line2x[0]=rnd(MAXX-MINX);
line2y[0]=rnd(MAXY-(MINY+OFFSET));

// Set all other trails to this value

for (count=1;count<= (TRAILS-1);count++ )
    {
    line1x[count] = line1x[0];
    line1y[count] = line1y[0];
    line2x[count] = line2x[0];
    line2y[count] = line2y[0];
    }

// Initial movement values (signed value)

dx1 = 2;
dy1 = 3;
dx2 =-3;
dy2 =-2;

// Queue variables

start = 0;
end   = (TRAILS-1);

// Title
#if defined(SAM)
sam_load_palette(fullcopper);
#endif
textcolor(TEXT);
textbackground(BACK);
gotoxy(0,0);
printf("TrailBlazer - Press key for next");

palcount =0;
moveon=0;

// Main loop

while (!keypress() &! moveon)
{
// Resolve pointer to the last line values and wrap if need be

last = (start+1);
if (last == TRAILS) last =0;

// Set 'New' values with co-ords +/- movement values from last's

new1x = line1x[last]+dx1;
new1y = line1y[last]+dy1;
new2x = line2x[last]+dx2;
new2y = line2y[last]+dy2;

// if 'New' co-ords over-range then invert the movement values

if (new1x <= MINX) dx1-= (dx1*2);
if (new2x <= MINX) dx2-= (dx2*2);
if (new1y <= (MINY+OFFSET)) dy1-= (dy1*2);
if (new2y <= (MINY+OFFSET)) dy2-= (dy2*2);
if (new1x >= MAXX) dx1-= (dx1*2);
if (new2x >= MAXX) dx2-= (dx2*2);
if (new1y >= MAXY) dy1-= (dy1*2);
if (new2y >= MAXY) dy2-= (dy2*2);

// Now safe to set front of trail to last + movement values

line1x[start] = line1x[last] + dx1;
line1y[start] = line1y[last] + dy1;
line2x[start] = line2x[last] + dx2;
line2y[start] = line2y[last] + dy2;

// Draw trail front

#if defined(SAM)
textcolor((palcount % 12)+1);
#endif
#if defined(SPECTRUM)
textcolor(TEXT);
#endif
draw(line1x[start],line1y[start],line2x[start],line2y[start]); 

// Blank out last line of trail

#if defined(SAM)
textcolor(0);
draw(line1x[end], line1y[end], line2x[end], line2y[end]); 
#endif
#if defined(SPECTRUM)
undraw(line1x[end], line1y[end], line2x[end], line2y[end]); 
#endif
if (palcount == 60000) palcount=0;
palcount++ ;

// Shift Queue pointers

end-- ;
start-- ;
if (end < 0) end = (TRAILS-1);
if (start < 0) start = (TRAILS-1);
}
moveon=0;
}

// Life, it's the name of the game

void life()
{
textcolor(15);
setuplife();
moveon=0;

while (!keypress() && popul)
	{
    gen++ ;
	gotoxy(0,23);
    printf("Generation:%4d Population:%3d",gen,popul);
    cyclelife();
    if (moveon || keypress()) break;
    outputlife();
    if (moveon || keypress()) break;
    }
moveon=0;
}

// Setup Life function

void setuplife()
{
int rnumber,i,row,col;

srand(LIFESEED);

gen=0;
rnumber=rnd(60)+60;
popul=rnumber;


gotoxy(0,22);
printf("Life - Press key for next part\n");
printf("Generation:%4d Population:%3d",gen,popul);

for (i=0; i < rnumber; i++ )
    {
    row=rnd(ROWS);
    col=rnd(COLS);
    world[row][col]='X'; // putchar in a cell
    gotoxy(col,row);
    putchar('O');
    }
}

// Update screen and world

void outputlife()
{
int row,col;
char cell;

popul=0;
for (row=0; row < ROWS; row++ )
    {
	for (col=0; col < COLS; col++ )
        {
        cell=world[row][col];
      if (cell && (cell == 3 || cell == 'X'+2 || cell == 'X'+3))
         {
         popul++ ;

         if (cell < 'X')
            {
            gotoxy(col,row);
            putchar('O');
            }
         cell='X';
         }
      else
         {
         if (cell >= 'X')
            {
            gotoxy(col,row);
            putchar(' ');
            }
         cell=0;
         }
      world[row][col]=cell;
      }
        }
}

// Generate next life cycle

void cyclelife()
{
int row,col;

// Take care of left and right column first

for (row=0; row < ROWS; row++ )
    {
    if (world[row][0] >= 'X') addd(row,0);
    if (world[row][COLS-1] >= 'X') addd(row,COLS-1);
    }

// Take care of top and bottom line

for (col=1; col < COLS-1;col++ )
    {
    if (world[0][col] >= 'X') addd(0,col);
    if (world[ROWS-1][col] >= 'X') addd(ROWS-1,col);
	if (keypress()) break;
    }

// Fill in the box, ignoring border conditions

for (row=1; row < ROWS-1; row++ )
    {
    for (col=1; col < COLS-1; col++ )
        {
        if (world[row][col] >= 'X' )
            {
            world[row-1][col-1]++ ;
            world[row-1][col]++ ;
            world[row-1][col+1]++ ;
            world[row][col-1]++ ;
            world[row][col+1]++ ;
            world[row+1][col-1]++ ;
            world[row+1][col]++ ;
            world[row+1][col+1]++ ;
			if (keypress()) break;
            }
        }
    }
}

// Addddddd

void addd(int row,int col)
{
int rrow,ccol,rr,cc;

for (rr=row-1; rr <= row+1; rr++ )
    {
    for (cc=col-1; cc <= col+1; cc++ )
        {
        rrow=rr != -1 ? rr : ROWS-1;
        ccol=cc != -1 ? cc : COLS-1;
        if (rrow >= ROWS) rrow=0;
        if (ccol >= COLS) ccol=0;
        world[rrow][ccol]++ ;
        }
    }
world[row][col]-- ;
}

// Outro section

void outro()
{
moveon=0;
pstartup();

while (!keypress())
    {
    outprinty(3,mess4);
    if (moveon) break ;
    outprinty(6,mess5);
    if (moveon) break ;
    outprinty(9,mess6);
    if (moveon) break ;
    outprinty(12,mess7);
    if (moveon) break ;
    outprinty(15,mess8);
    if (moveon) break ;
    outprinty(18,mess9);

    while (!moveon)
        {
        fadedown(11,copper);
        if (keypress()) break ;
		fadedown(12,red);
		if (keypress()) break ;
        fadedown(13,blue);
		if (keypress()) break ;
        fadedown(14,grey);
		if (keypress()) break ;
        fadeup(11,copper);
		if (keypress()) break ;
        fadeup(12,red);
		if (keypress()) break ;
        fadeup(13,blue);
		if (keypress()) break ;
        fadeup(14,grey);
        }
    }
moveon=0;
}

// Startup routine for parallax stars

void pstartup()
{
textcolor(1);
for (count=0;count<= (PSTARS-1);count++ )
    {
    pstarx[count] = rnd(255);
    pstary[count] = rnd(191);
    pstars[count] = (count % 3)+1;

    if (!point(pstarx[count],pstary[count]))
        {
        textcolor(pstars[count]);
        plot(pstarx[count],pstary[count]);
        }
    }
}

// Move stars

void psmove()
{
intrinsic_halt();	// Wait for start of next frame
for (count=0;count<= (PSTARS-1);count++ )
    {
    if (point(pstarx[count],pstary[count]) < 4)
        {
        textcolor(0);
        plot(pstarx[count],pstary[count]);
        }

    pstarx[count]+= pstars[count];
    if (pstarx[count] > 255) pstarx[count]=0;

    if (!point(pstarx[count],pstary[count]))
        {
        textcolor(pstars[count]);
        plot(pstarx[count],pstary[count]);
        }
    }
}

// Routine for printing outro texts

void outprinty(int lineno, char *message)
{
int x;

gotoxy((32-strlen(message))/2,lineno);
for (x=0;x< strlen(message);x++ )
    {
    textcolor((x % 4)+11);
    printf("%c",message[x]);
	if (keypress()) break ;
    psmove();
    }
}

// Outro Fade down

void fadedown(int colour, int *colset)
{
int x;
#if defined(SAM)
for (x=0;x<5;x++ )
    {
    sam_set_palette(colour,colset[x]);
    psmove();
    if (keypress()) break ;
    }
#endif
}

// Outro Fade up

void fadeup(int colour, int *colset)
{
int x;
#if defined(SAM)
for (x=4;x>0;x-- )
    {
    sam_set_palette(colour,colset[x]);
    psmove();
    if (keypress()) break ;
    }
#endif
}

