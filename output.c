/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"
#if 0
char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char e_header[256];
char e_line[256];
char e_cells[256];

void initboard_format(void)
{
    int i;
    
    strcpy(e_header, "   ");
    strcpy(e_line,   "   ");
    strcpy(e_cells,  " %d ");
    
    for (i = 0; i < BOARDSIZE; i++) {
        strcat(e_header, "  ");
        strncat(e_header, alpha + i, 1);
        strcat(e_header, " ");
        strcat(e_line, "+---");
        strcat(e_cells, "| %c ");
    }
    strcat(e_header, "\\n");
    strcat(e_line,   "+\\n");
    strcat(e_cells,  "|\\n");
    
    printf("%s\n", e_header);
    printf("%s\n", e_line);
    printf("%s\n", e_cells);
    
}
#endif


char *header = "     A   B   C   D   E   F   G   H\n";
char *line   = "   +---+---+---+---+---+---+---+---+\n";
char *cells  = " %d | %c | %c | %c | %c | %c | %c | %c | %c |\n";
char stone[3] = {'O', ' ', '@'};

int output(struct board *bd)
{
    int x, y;
    
    printf(header);
    printf(line);
    for (y = 0; y < BOARDSIZE; y++) {
        printf(cells,
               y + 1,
               stone[1 + (int)bd->b[0][y]],
               stone[1 + (int)bd->b[1][y]],
               stone[1 + (int)bd->b[2][y]],
               stone[1 + (int)bd->b[3][y]],
               stone[1 + (int)bd->b[4][y]],
               stone[1 + (int)bd->b[5][y]],
               stone[1 + (int)bd->b[6][y]],
               stone[1 + (int)bd->b[7][y]]
               );
        printf(line);
    }
    
}


