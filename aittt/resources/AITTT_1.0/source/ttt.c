/*
 * A.I. Tic Tac Toe - a simple Tic Tac Toe game
 *
 * Copyright 2005 Marcello Zaniboni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#if defined __SPECTRUM__
#include <sound.h>
#include <graphics.h>
#endif

#define BOARD_SIZE 9

/* Graphical stuffs (res: 256 x 192) */
#define BOARD_POSITION_X 33
#define BOARD_POSITION_Y 26
#define CELL_SIZE 22

/* turns degug prints on/off */
#define DEBUG_LOGGING 0

/* the size of a saved move and their number */
#define MOVE_LEN 10
#define MOVE_MEMORY 150

#define DRAW_GAME 99

/* The length for some output strings */
#define OUT_STR_LEN 15

/* Returns the correct char for each player */
#define PLAYER_SIGNATURE(pl) (((pl) == 0) ? " " : (((pl) == 1) ? "O" : "X"))


/* prototipes */
int getUserInput(void);
void cleanScreen(void);
void cleanScreenLight(void);
void appMenu(void);
void storeMatch(int);
void redrawBoard(unsigned char board[]);
double percentage(unsigned int, unsigned int);
void doubletrunc(char *, int, double);
int getHumanMove(int player, unsigned char board[]);
int checkObviousMove(int player, unsigned char board[]);
int computeMove(int player, unsigned char board[]);
int testVictory(unsigned char board[]);
void saveCurrentMove(unsigned char board[], int move);
void setwhiteonblack(void);
void setblackonwite(void);
void setgreenonblack(void);
void sleephalfsecond(void);
void waitForKeyPress(void);
void printAt(int, int, char *);
void printIntAt(int, int, int);

/*
  The following two arrays contain the computer
  moves fir the games in which the human won.
  The data are in the following format.
    10 bytes per move (MOVE_LEN):
    * the first 9 bytes contain the board map
      with 0 for free cells, 1 or 2 for the
      cells occupied by the players, 11 or 12
      for the choosed move (10 + the player
      number).
    * the last byte is the number of the moves
      of the whole game.
*/
unsigned char *g_allSavedMoves;
unsigned int g_numberOfAllSavedMoves = 0;

/* current match moves */
unsigned char *g_savedMoves;
unsigned int g_numberOfSavedMoves = 0;

/* human player */
int g_humanPlayer;

/* total score variables */
int humanvictories = 0;
int computervictories = 0;


/* wait for a key */
void waitForKeyPress() {
    #if defined __SPECTRUM__
    while (!getkey());
    #endif
}

void sleephalfsecond() {
    #if defined __SPECTRUM__
    long start = clock();  
    long end = CLOCKS_PER_SEC / 2;
    while ((clock() - start) < end);
    #endif
}

/* color settings */
void setwhiteonblack() {
    #if defined __SPECTRUM__
    // ink white
    putchar(16);
    putchar(48);
    // paper black
    putchar(17);
    putchar(55);
    // bright off
    putchar(19);
    putchar(48);
    #endif
}

/* color settings */
void setblackonwite() {
    #if defined __SPECTRUM__
    // ink black
    putchar(16);
    putchar(48);
    // paper white
    putchar(17);
    putchar(55);
    // bright on
    putchar(19);
    putchar(49);
    #endif
}

/* color settings */
void setgreenonblack() {
    #if defined __SPECTRUM__
    // ink green
    putchar(16);
    putchar(52);
    // paper black
    putchar(17);
    putchar(48);
    // bright on
    putchar(19);
    putchar(49);
    #endif
}

double percentage(unsigned int total, unsigned int part) {
    double t = (double) total;
    double result = (double) 100.0 * ((double) part / t);
    return result;
}

void doubletrunc(char *output, int length, double value) {
    int i = 0, found = 0, decimal = 0;
    sprintf(output, "%f", value);
    for (i = 0; i < length; i++) {
        if (found) decimal++;
        if (output[i] == '.') found = 1;
        if (decimal > 1) output[i] = '\0';
    }
}

/* print a string at column c and row r */
void printAt(int c, int r, char *string) {
    #if defined __SPECTRUM__
        putchar(22);
        putchar(32 + r);
        putchar(32 + c);
        printf("%s", string);
    #else
        printf("\nFIX ME! (\"%s\")\n", string);
    #endif
}

/* print an int at column c and row r */
void printIntAt(int c, int r, int value) {
    #if defined __SPECTRUM__
        signed char buffer[5];
        itoa(buffer, value,5);
        printAt(c, r, buffer);
    #else
        printf("\nFIX ME! (%d)\n", value);
    #endif
}

/* read the user input */
int getUserInput() {
    #if defined __SPECTRUM__
        int k;
        while (!(k = getkey()));
        return k;
    #else
        int i;
        char c[OUT_STR_LEN];
        for (i = 0; i < OUT_STR_LEN; i++) c[i] = '\0';
        fgets(c, OUT_STR_LEN, stdin);
        return c[0];
    #endif
}

/* clear screen */
void cleanScreen(void) {
    #if defined __SPECTRUM__
        //clga(BOARD_POSITION_X - 1, BOARD_POSITION_Y - 1, 120, 71); /* screen */
        //clga(176, 25, 55, 54); /* baloon */
    #else
        /*
        int i;
        for (i = 0; i < 25; i++) printf("\n");
        */
        printf("\n\n---------------------------------------\n\n");
    #endif
}

/* clear screen, except for the board */
void cleanScreenLight(void) {
    #if defined __SPECTRUM__
        int boardSize;
        boardSize = CELL_SIZE * 3 + 1;
       // clga(BOARD_POSITION_X + boardSize, BOARD_POSITION_Y - 1, 120 - boardSize - 2, 71); /* text only */
       // clga(176, 25, 55, 54); /* baloon */
    #else
        printf("\n- - - - -\n\n");
    #endif
}

/* board (re)draw */
void redrawBoard(unsigned char board[]) {
    int i, x, y, r;

    #if defined __SPECTRUM__
        cleanScreenLight();

        /* board draw */
        drawb(BOARD_POSITION_X, BOARD_POSITION_Y, CELL_SIZE * 3 + 1, CELL_SIZE * 3 + 1);
        draw(BOARD_POSITION_X + CELL_SIZE, BOARD_POSITION_Y, BOARD_POSITION_X + CELL_SIZE, BOARD_POSITION_Y + (CELL_SIZE * 3 - 1));
        draw(BOARD_POSITION_X + CELL_SIZE * 2, BOARD_POSITION_Y, BOARD_POSITION_X + CELL_SIZE * 2, BOARD_POSITION_Y + (CELL_SIZE * 3 - 1));
        draw(BOARD_POSITION_X, BOARD_POSITION_Y + CELL_SIZE, BOARD_POSITION_X + (CELL_SIZE * 3 - 1), BOARD_POSITION_Y + CELL_SIZE);
        draw(BOARD_POSITION_X, BOARD_POSITION_Y + CELL_SIZE * 2, BOARD_POSITION_X + (CELL_SIZE * 3 - 1), BOARD_POSITION_Y + CELL_SIZE * 2);
    
        /* pieces draw */
        for (i = 0; i < BOARD_SIZE; i++) {
            if (board[i] != 0) {
                int halfsize = CELL_SIZE / 2 - 2;
                y = (i / 3) * CELL_SIZE + BOARD_POSITION_Y + CELL_SIZE / 2;
                x = (i % 3) * CELL_SIZE + BOARD_POSITION_X + CELL_SIZE / 2;
                if (board[i] == 1)
                    circle(x, y, halfsize, 1);
                else {
                    /* for (r = CELL_SIZE / 2 - 2; r > 0; r--) circle(x, y, r, 1);*/
                    halfsize--;
                    draw(x - halfsize, y - halfsize, x + halfsize, y + halfsize);
                    draw(x - halfsize, y + halfsize, x + halfsize, y - halfsize);
                }
            }
        }
    #else
        printf("Board status:\n");
        printf("+---+---+---+   Board map:\n");
        printf("| %s | %s | %s |    1 | 2 | 3 \n",
            PLAYER_SIGNATURE(board[0]),
            PLAYER_SIGNATURE(board[1]),
            PLAYER_SIGNATURE(board[2]));
        printf("+---+---+---+   ---+---+---\n");
        printf("| %s | %s | %s |    4 | 5 | 6 \n",
            PLAYER_SIGNATURE(board[3]),
            PLAYER_SIGNATURE(board[4]),
            PLAYER_SIGNATURE(board[5]));
        printf("+---+---+---+   ---+---+---\n");
        printf("| %s | %s | %s |    7 | 8 | 9 \n",
            PLAYER_SIGNATURE(board[6]),
            PLAYER_SIGNATURE(board[7]),
            PLAYER_SIGNATURE(board[8]));
        printf("+---+---+---+\n");
    #endif
}

/* get user input for the choosen move */
int getHumanMove(int player, unsigned char board[]) {
    int position, x;
    int notOk = -1;
    while (notOk) {
        x = 0;
        #if defined __SPECTRUM__
            setgreenonblack();
            printAt(26, 4, "Player");
            printIntAt(33, 4, player);
            printAt(26, 5, "your move?");
            printAt(26, 6, "(1 to 9)");
            setblackonwite();
            printAt(45, 3, "Board map:");
            printAt(45, 5, " 1 | 2 | 3 ");
            printAt(45, 6, "---+---+---");
            printAt(45, 7, " 4 | 5 | 6 ");
            printAt(45, 8, "---+---+---");
            printAt(45, 9, " 7 | 8 | 9 ");
        #else
            printf("Player %d, your move (1-9) ? ", player);
        #endif
        x = getUserInput() - '0';
        if (x < 1 || x > BOARD_SIZE) {
            /*
            printf("\nWRONG POSITION.\n\n");
            printKeybInfo();
            */
        } else {
            position = x - 1;
            if (board[position] == 0) {
                board[position] = player;
                notOk = 0;
            } else {
                /*
                printf("\nTHE CELL IS YET OCCUPIED\nBY PLAYER %d!\n\n", board[position]);
                printKeybInfo();
                */
            }
        }
    }
    setgreenonblack();
    #if defined __SPECTRUM__
        printIntAt(35, 6, x);
    #else
        printf("\n");
    #endif
    sleephalfsecond();
    return position;
}

/* try to predict any obvious victory (and avoid it if human's) */
int checkObviousMove(int player, unsigned char board[]) {
    int i, j;

    for (j = 0; j < 2; j++) {
        for (i = 0; i < 3; i++) {
    
            /* three vertical lines */
            if (board[i] == player && board[i] == board[i + 3] && board[i + 6] == 0) return i + 6;
            if (board[i] == player && board[i] == board[i + 6] && board[i + 3] == 0) return i + 3;
            if (board[i + 3] == player && board[i + 3] == board[i + 6] && board[i] == 0) return i;
    
            /* three horizontal lines */
            if (board[3 * i] == player && board[3 * i] == board[3 * i + 1] && board[3 * i + 2] == 0) return 3 * i + 2;
            if (board[3 * i] == player && board[3 * i] == board[3 * i + 2] && board[3 * i + 1] == 0) return 3 * i + 1;
            if (board[3 * i + 1] == player && board[3 * i + 1] == board[3 * i + 2] && board[3 * i] == 0) return 3 * i;
        }
    
        /* first diagonal */
        if (board[0] == player && board[0] == board[4] && board[8] == 0) return 8;
        if (board[0] == player && board[0] == board[8] && board[4] == 0) return 4;
        if (board[4] == player && board[4] == board[8] && board[0] == 0) return 0;
    
        /* second diagonal */
        if (board[2] == player && board[2] == board[4] && board[6] == 0) return 6;
        if (board[2] == player && board[2] == board[6] && board[4] == 0) return 4;
        if (board[4] == player && board[4] == board[6] && board[2] == 0) return 2;

        /* after having tried to win, try to block any victory of the human player */
        player = 3 - player; /* for the next turn */
    }
    return 0;
}

/* computer move */
int computeMove(int player, unsigned char board[]) {
    int i, j, bestMoveRank, returnValue;
    int sameSituation;
    int obvious;                /* an obvious choice */
    int move[BOARD_SIZE];       /* valid moves */
    int moveRank[BOARD_SIZE];   /* valid move ranks */
    int numberOfValidMoves = 0;

    #if defined __SPECTRUM__
        setblackonwite();
        printAt(45, 4, "Thinking...");
        sleephalfsecond();
    #endif

    /* STEP 0: watch for obvious moves */
    obvious = checkObviousMove(player, board);
    if (!obvious) {

        /* STEP 1: look for the possible set of moves on
           the board and assign them the maximum rank */
        for (i = 0; i < BOARD_SIZE; i++) {
            move[i] = 0;
            moveRank[i] = 255;
            if (board[i] == 0) {
                move[numberOfValidMoves] = i;
                numberOfValidMoves++;
            }
        }
    
        /* STEP 2: search the saved moves for this situation
            and correct the move rank of the valid move set */
        if (DEBUG_LOGGING) printf("\nsearching same situation: 0 to %d...\n", g_numberOfAllSavedMoves);
        for (i = 0; i < g_numberOfAllSavedMoves; i++) { /* loop the moves */
            sameSituation = 1;
            for (j = 0; j < BOARD_SIZE; j++) { /* loop the board cells */
                if (g_allSavedMoves[i * MOVE_LEN + j] <= 2
                    && g_allSavedMoves[i * MOVE_LEN + j] != board[j]) {
                    sameSituation = 0;
                    break;
                }
            }
    
            if (sameSituation) {
    
                /* the saved move is useful, let's get its rank... */
                int newRank,
                    theSavedMove;
                for (j = 0; j < BOARD_SIZE; j++)
                    if (g_allSavedMoves[i * MOVE_LEN + j] > 9) {
                        theSavedMove = j;
                        break;
                    }
                newRank = g_allSavedMoves[i * MOVE_LEN + BOARD_SIZE];
                if (DEBUG_LOGGING) printf("FOUND: move=%d, rank=%d, abs_idx=%d\n", (theSavedMove + 1), newRank, (i * MOVE_LEN + j));
    
                /* ...and (if necessary) decrease the corresponding value in moveRank[] */
                for (j = 0; j < numberOfValidMoves; j++)
                    if (move[j] == theSavedMove)
                        moveRank[j] = (moveRank[j] > newRank) ? newRank : moveRank[j];
            }
        }
    
        /* STEP 4: choose one of the valid moves with the hiest rank */
        bestMoveRank = 0;
        for (i = 0; i < numberOfValidMoves; i++) /* find the best rank */
            bestMoveRank = (moveRank[i] > bestMoveRank) ? moveRank[i] : bestMoveRank;
        for (i = 0; i < numberOfValidMoves; i++) { /* find the first best move */
            if (moveRank[i] == bestMoveRank) {
                returnValue = move[i];
                break;
            }
        }
    } else {
        #if defined __SPECTRUM__
            printAt(45, 5, "...obvious:");
            printAt(45, 7, "(there are 2");
            printAt(45, 8, "in a row!)");
        #else
            printf("...obvious: there are 2 in a row!\n");
        #endif
            returnValue = obvious;
    }

    /* STEP 5: set the move on the board */
    board[returnValue] = player;

    #if defined __SPECTRUM__
        printIntAt(47, 6, returnValue + 1);
        sleep(1);
    #else
        printf("A.I. move: %d\n\n", returnValue + 1);
    #endif

    return returnValue;
}

/**********************************************************
Return 0 if there isn't yet a winner, 1 or 2 if there is or
DRAW_GAME if the match ended without a winner.
**********************************************************/
int testVictory(unsigned char board[]) {
    int i;
    int draw = 1;

    /************
      board map:
        0 1 2
        3 4 5
        6 7 8
    ************/

    /* horizontal lines */
    if (board[0] > 0 && board[0] == board[1] && board[1] == board[2]) return board[0];
    if (board[3] > 0 && board[3] == board[4] && board[4] == board[5]) return board[3];
    if (board[6] > 0 && board[6] == board[7] && board[7] == board[8]) return board[6];

    /* vertical lines */
    if (board[0] > 0 && board[0] == board[3] && board[3] == board[6]) return board[0];
    if (board[1] > 0 && board[1] == board[4] && board[4] == board[7]) return board[1];
    if (board[2] > 0 && board[2] == board[5] && board[5] == board[8]) return board[2];

    /* diagonal lines */
    if (board[0] > 0 && board[0] == board[4] && board[4] == board[8]) return board[0];
    if (board[2] > 0 && board[2] == board[4] && board[4] == board[6]) return board[2];

    /* draw */
    for (i = 0; i < 9 ; i++) if (board[i] == 0) draw = 0;
    if (DEBUG_LOGGING && draw) {
        printf("DRAW GAME: ");
        for (i = 0; i < 9 ; i++) printf("%d", board[i]);
        printf("\n");
        sleep(4);
    }
    if (draw) return DRAW_GAME;

    /* no victory (yet) */
    return 0;
}

/* main menu */
void appMenu(void) {
    int human = 0;
    #if defined __SPECTRUM__
        setwhiteonblack();
        printAt(3, 22, "AI TIC TAC TOE - (C) 2005 Marcello Zaniboni - ver. 1.0");
        setgreenonblack();
        printAt(9, 3, "(C) 2005 Marcello Zaniboni");
        printAt(9, 4, "This program comes with");
        printAt(9, 5, "ABSOLUTELY NO WARRANTY.");
        printAt(9, 6, "You may redistribute copies");
        printAt(9, 7, "of this program under the");
        printAt(9, 8, "terms of the GNU General");
        printAt(9, 9, "Public License.");
        printAt(9, 10, "[press any key to continue]");
        setblackonwite();
        printAt(45, 4, "For a copy");
        printAt(45, 5, "of the GNU");
        printAt(45, 6, "GPL see the");
        printAt(45, 7, "next track");
        printAt(45, 8, "on the tape");
        waitForKeyPress();
        cleanScreen();
        setgreenonblack();
        printAt(9, 3, "This game will show you");
        printAt(9, 4, "how the computer can learn");
        printAt(9, 5, "playing Tic Tac Toe by its");
        printAt(9, 6, "own mistakes (looking at");
        printAt(9, 7, "the past games).");
        printAt(9, 8, "      mzaniboni@hotmail.com");
        printAt(9, 10, "[press any key to continue]");
        setblackonwite();
        printAt(45, 4, "Graphics:");
        printAt(45, 5, " Luciano");
        printAt(45, 6, " \"Lucky\"");
        printAt(45, 7, " Costarelli");
        waitForKeyPress();
        cleanScreen();
    #else
        printf("AI TIC TAC TOE\n(C) 2005 Marcello Zaniboni - v. 1.0\n");
        printf("mzaniboni@hotmail.com\n\n");
        printf("This program comes with ABSOLUTELY NO\n");
        printf("WARRANTY. You may redistribute copies\n");
        printf("of this program under the terms of the\n");
        printf("GNU General Public License.\n\n");
        printf("This game will show you how the\n");
        printf("computer can learn playing Tic Tac\n");
        printf("Toe by its own mistakes (looking\n");
        printf("at the past games).\n\n");
    #endif
    while (human != 1 && human != 2) {
        #if defined __SPECTRUM__
            setgreenonblack();
            printAt(9, 4, "Player 1 starts first.");
            printAt(9, 5, "Which player do you choose?");
            printAt(9, 7, "1 or 2?");
            setblackonwite();
            printAt(45, 5, "Player 1: O");
            printAt(45, 7, "Player 2: X");
        #else
            printf("Player 1 starts first.\n");
            printf("Player 1: O, player 2: X\n");
            printf("Which player do you choose (1 or 2)?\n");            
        #endif
        human = getUserInput() - '0';
    }
    g_humanPlayer = human;
    #if defined __SPECTRUM__
        setgreenonblack();
        printIntAt(18, 7, human);
        sleephalfsecond();
    #endif
}

/**********************************************************
The computer move is saved in a temporary array: if the
human will win, this array (containing the current match
moves) will be saved in the main game history array by
another function. The move is identified by a +10 factor.
**********************************************************/
void saveCurrentMove(unsigned char board[], int move) {
    int i;
    int boardCounter = 0;
    for (i = g_numberOfSavedMoves * MOVE_LEN;
        i < MOVE_LEN + g_numberOfSavedMoves * MOVE_LEN - 1;
        i++) {
            g_savedMoves[i] = board[boardCounter]
                + ((move == i % MOVE_LEN) ? 10 : 0);
            boardCounter++;
        }
    g_numberOfSavedMoves++;
}


/**********************************************************
The computer lost. Its moves are saved into the main array
containing all the computer past experience: a set of moves
and game duration (expressed in number of moves).
**********************************************************/
void storeMatch(int moveCounter) {
    int i;
    for (i = 0; i < g_numberOfSavedMoves * MOVE_LEN; i++) {
        g_allSavedMoves[g_numberOfAllSavedMoves * MOVE_LEN + i] =
            ((i + 1) % 10)  ? g_savedMoves[i] : moveCounter;
    }
    g_numberOfAllSavedMoves += g_numberOfSavedMoves;
    if (DEBUG_LOGGING) {
        printf("\nComputer experience summary\n  - ");
        for (i = 0; i < g_numberOfAllSavedMoves * MOVE_LEN; i++) {
            printf("%d ", g_allSavedMoves[i]);
            if (!((i + 1) % MOVE_LEN)) printf("\n  - ");
        }
        printf("n = %d\n", g_numberOfAllSavedMoves);
    }
}

int main(void) {
    int player, winner, i, moveCounter, move, startNewGame;
    /* memory allocation */
    unsigned char board[BOARD_SIZE];
    unsigned char allmoves[MOVE_LEN * MOVE_MEMORY];
    unsigned char curmoves[MOVE_LEN * 5];
    
    #if defined __SPECTRUM__
        cleanScreen();
    #endif
    appMenu();
    #if defined __SPECTRUM__
        cleanScreen();
    #endif


    /* initialisation */
    for (i = 0; i < MOVE_LEN * MOVE_MEMORY; i++) allmoves[i] = 0;
    g_allSavedMoves = &allmoves[0];
    g_savedMoves = &curmoves[0];
    g_numberOfAllSavedMoves = 0;

    /* game */
    startNewGame = 1;
    while (startNewGame) {
        char scoremsg[OUT_STR_LEN];

        /* match init */
        cleanScreenLight();
        player = 1;
        winner = 0;
        for (i = 0; i < BOARD_SIZE; i++) board[i] = 0;
        for (i = 0; i < MOVE_LEN * 5; i++) /* max 5 moves per game */
            g_savedMoves[i] = 0;
        redrawBoard(board);

        /* match*/
        moveCounter = 0;
        g_numberOfSavedMoves = 0;
        while (!(winner = testVictory(board))) {
            move = (player == g_humanPlayer)
                ? getHumanMove(player, board)
                : computeMove(player, board);
            redrawBoard(board);
            #if defined __SPECTRUM__
                bit_fx((player == g_humanPlayer) ? 0 : 1);
            #endif
            moveCounter++;
            if (player != g_humanPlayer) saveCurrentMove(board, move);
            player = 3 - player; /* for the next turn */
        }
        #if defined __SPECTRUM__
            cleanScreenLight();
        #endif
        if (winner != DRAW_GAME) {
            #if defined __SPECTRUM__
                setgreenonblack();
                printAt(26, 5, "Player");
                printIntAt(33, 5, winner);
                printAt(26, 6, "wins!!!");
            #else
                printf("\nPLAYER %d WINS!!!\n", winner);
            #endif
            if (DEBUG_LOGGING) {
                printf("Match debug info:\n  - ");
                for (i = 0; i < g_numberOfSavedMoves * MOVE_LEN; i++) {
                    printf("%d ", g_savedMoves[i]);
                    if (!((i + 1) % MOVE_LEN)) printf("\n  - ");
                }
                printf("n = %d\n", g_numberOfSavedMoves);
                sleephalfsecond();
            }
            if (winner == g_humanPlayer) {
                double perc;
                char memusage[OUT_STR_LEN];
                char memuoutput[OUT_STR_LEN];
                storeMatch(moveCounter);
                humanvictories++;
                perc = percentage(MOVE_MEMORY, MOVE_MEMORY - g_numberOfAllSavedMoves);
                doubletrunc(memusage, OUT_STR_LEN, perc);
                sprintf(memuoutput, " %s%%\0", memusage);
                #if defined __SPECTRUM__
                    setblackonwite();
                    printAt(45, 4, "Number of");
                    printAt(45, 5, "stored");
                    printAt(45, 6, "moves:");
                    printIntAt(53, 6, g_numberOfAllSavedMoves);
                    printAt(45, 7, "Free memory:");
                    printAt(47, 8, memuoutput);
                    bit_fx(7);
                    bit_fx(7);
                #else
                    printf("   A.I. info:\n");
                    printf("   stored moves: %d\n", g_numberOfAllSavedMoves);
                    printf("   free memory: %s\n", memuoutput);
                #endif
            } else {
                computervictories++;
                #if defined __SPECTRUM__
                    setblackonwite();
                    printAt(45, 5, "You loose!");
                    bit_fx(2);
                    bit_fx(2);
                    bit_fx(2);
                    bit_fx(2);
                #else
                    printf("You loose!\n");
                #endif
            }
        } else {
            double perc;
            char memusage[OUT_STR_LEN];
            char memuoutput[OUT_STR_LEN];
            setblackonwite();
            perc = percentage(MOVE_MEMORY, MOVE_MEMORY - g_numberOfAllSavedMoves);
            doubletrunc(memusage, OUT_STR_LEN, perc);
            sprintf(memuoutput, " %s%%\0", memusage);
            #if defined __SPECTRUM__
                setblackonwite();
                printAt(45, 4, "Number of");
                printAt(45, 5, "stored");
                printAt(45, 6, "moves:");
                printIntAt(53, 6, g_numberOfAllSavedMoves);
                printAt(45, 7, "Free memory:");
                printAt(47, 8, memuoutput);
                setgreenonblack();
                bit_fx(6);
                printAt(26, 5, "Nobody");
                printAt(26, 6, "wins...");
            #else
                printf("Nobody wins...\n");
                printf("   A.I. info:\n");
                printf("   stored moves: %d\n", g_numberOfAllSavedMoves);
                printf("   free memory: %s\n", memuoutput);
            #endif
        }
        for (i = 0; i < OUT_STR_LEN; i++) scoremsg[i] = '\0';
        sprintf(scoremsg, "Human: %d", humanvictories);
        #if defined __SPECTRUM__
            setgreenonblack();
            printAt(26, 8, scoremsg);
            for (i = 0; i < OUT_STR_LEN; i++) scoremsg[i] = '\0';
            sprintf(scoremsg, "A.I.: %d", computervictories);
            printAt(26, 9, scoremsg);
            sleep(5);
            cleanScreen();
        #else
            printf("%s\n", scoremsg);
            for (i = 0; i < OUT_STR_LEN; i++) scoremsg[i] = '\0';
            sprintf(scoremsg, "A.I.: %d", computervictories);
            printf("%s\n", scoremsg);
            printf("Press return to continue.");
            getUserInput();
        #endif
    }
    
    while (1) { sleep(60); }

    return 0;

}
