/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"

char *header = "     A   B   C   D   E   F   G   H\n";
char *line   = "   +---+---+---+---+---+---+---+---+\n";
char *cells  = " %d | %c | %c | %c | %c | %c | %c | %c | %c |\n";
char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char stone[3] = {' ', '@', 'O'};

int output(struct board *bp)
{
    int x, y;
    
    printf("    ");
    for (x = 0; x < bp->xsize; x++) {
        printf("  %c ", alpha[x]);
    }
    printf("\n");
    printf("    +");
    for (x = 0; x < bp->xsize; x++) {
        printf("---+");
    }
    printf("\n");    
    for (y = 0; y < bp->ysize; y++) {
        printf(" %2d |", y + 1);
        for (x = 0; x < bp->xsize; x++) {
            printf(" %c |", stone[1 + CELL((*bp), x, y)]);
        }
        printf("\n");
        printf("    +");
        for (x = 0; x < bp->xsize; x++) {
            printf("---+");
        }
        printf("\n");        
    }
}
