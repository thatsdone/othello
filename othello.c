/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "othello.h"


struct queue top;

struct board bd;


int getpoint(char *buf, struct point *p)
{
    int len;
    
    if (isalpha(buf[0]) == 0) {
        return NO;
    }
    p->x = ((int)toupper(buf[0])) - ((int)'A');
    dprintf("x is %d\n", p->x);
    len = strlen(&(buf[1]));
    p->y = atoi(&(buf[1])) - 1;
    dprintf("y value length/value is %d/%d\n", len, p->y);
    dprintf("Input is x=%d/y=%d\n", p->x, p->y);
    
    if ((p->x >= BOARDSIZE) || (p->y >= BOARDSIZE) || (p->y < 0)) {
        return NO;
    }
    return YES;
}


int serve_computer_turn(struct board *bp, int color)
{
    int ret;
    struct put *putp;
    
    putp = allocput();
    printf("Thinking...\n");
    ret = think(&bd, putp, color);
    if (ret != YES) {
        printf("Pass!\n");
        putp->color = color;
        putp->p.x = -1;
        putp->p.y = -1;        
        append(&top, &(putp->main));
        return PASS;
    }
    append(&top, &(putp->main));
    ret = check_puttable(bp, putp, color);
    if (ret == YES) {
        bd.b[putp->p.x][putp->p.y] = color;
    } else {
        printf("internal inconsistency\n");
        exit(255);
    }
    dprintf("check_puttable returns %d\n", ret);
    ret = process_put(&bd, putp, color);
    if (ret < 0) {
        printf("internal inconsistency\n");
        exit(255);
    }
    return SERVED;
}

static int counter = 0;
int increment_cell_num(void)
{
    counter++;

    if (counter == (BOARDSIZE * BOARDSIZE)) {
        return YES;
    } else {
        return NO;
    }
}

int get_occupied_cell_num(void)
{
    return counter;
}



void finalize(void)
{
    printf("Game Over!\n");
}



int interactive(struct board *b)
{
    char buf[256];
    struct put *putp;
    int ret;
    int color;
    int turn = 0;
    int occupied_cell = 0;
    int is_end = NO;
    int was_pass;
    
    output(&bd);

    color = BLACK;

    if ((mode == MODE_HUMAN_COMPUTER) && (serve_first == YES)) {
        ret = serve_computer_turn(&bd, color);
        if (ret != PASS) {
            increment_cell_num();
            was_pass = YES;
        }
        /* flip color */
        color = OPPOSITE_COLOR(color);
        output(&bd);
    }
    
    putp = NULL;
    while (1) {
        /* allocate putp, if necessory */
        if (putp == NULL){
            putp = allocput();
        }
        /* display prompt */
        if (color == BLACK) {
            printf("Command(BLACK): ");
        } else {
            printf("Command(WHITE): ");
        }
        /* get user input */
        if (fgets(buf, 256, stdin) == NULL) {
            break;
        }
        /* chop */
        buf[strlen(buf) - 1] = 0x00;
        dprintf("get '%s'\n", buf);

        if (strcmp(buf, "show") == 0) {
            /*
             * show command
             */
            dprintf("command 'show' accepted.\n");
            command_show();
            
        } else if (strcmp(buf, "pass") == 0) {
            /*
             * pass command
             */
            dprintf("command 'pass' accepted.\n");
            putp->color = color;
            putp->p.x = -1;
            putp->p.y = -1;            
            append(&top, &(putp->main));
            color = OPPOSITE_COLOR(color);
            putp = NULL;
            if (was_pass == YES) {
                finalize();
            }
            was_pass = YES;
            
        } else {
                /*
                 * normal input
                 */
            if(getpoint(buf, &putp->p) != YES) {
                printf("Input Error\n");
                output(&bd);
                goto next_turn;
                
            } else {
                ret = check_puttable(&bd, putp, color);
                dprintf("check_puttable returns %d\n", ret);
                if (ret == YES) {
                    bd.b[putp->p.x][putp->p.y] = color;
                    putp->color = color;
                    append(&top, &(putp->main));

                    ret = process_put(&bd, putp, color);
                    if (ret < 0) {
                        dprintf("internal inconsistency\n");
                        exit(255);
                    }
                    is_end = increment_cell_num();
                    if (is_end == YES) {
                        finalize();
                    }
                    putp = NULL;
                        /* flip color */
                    color = OPPOSITE_COLOR(color);
                    was_pass = NO;
                
                } else {
                    printf("Input error\n");
                    output(&bd);
                    goto next_turn;
                }
            }
            output(&bd);
            if (is_end == YES) {
                finalize();
            }
            if (mode == MODE_HUMAN_COMPUTER){
                ret = serve_computer_turn(&bd, color);
                /* flip color */
                output(&bd);
                if (ret != PASS) {
                    is_end = increment_cell_num();
                    if (is_end == YES) {
                        finalize();
                    }
                    was_pass = NO;
                } else {
                    if (was_pass == YES) {
                        finalize();
                    }
                    was_pass = YES;
                }
                color = OPPOSITE_COLOR(color);
            }
          next_turn:
            
        }
    }
}

    
int initboard(struct board *bp)
{
    int x, y;
    struct put *putp;
    
    for (x = 0; x < BOARDSIZE; x++) {
        for (y = 0; y < BOARDSIZE; y++) {
            bp->b[x][y] = EMPTY;
        }
    }
    bp->b[BOARDSIZE / 2 - 1][BOARDSIZE / 2 - 1] = WHITE;
    increment_cell_num();
    putp = allocput();
    putp->color = WHITE;
    putp->p.x = BOARDSIZE / 2 - 1;
    putp->p.y = BOARDSIZE / 2 - 1;
    append(&top, &(putp->main));
    
    bp->b[BOARDSIZE / 2 ][BOARDSIZE / 2] = WHITE;
    increment_cell_num();
    putp = allocput();
    putp->color = WHITE;
    putp->p.x = BOARDSIZE / 2;
    putp->p.y = BOARDSIZE / 2;
    append(&top, &(putp->main));

    bp->b[BOARDSIZE / 2 - 1][BOARDSIZE / 2] = BLACK;
    increment_cell_num();
    putp = allocput();
    putp->color = BLACK;
    putp->p.x = BOARDSIZE / 2 - 1;
    putp->p.y = BOARDSIZE / 2;
    append(&top, &(putp->main));
    
    bp->b[BOARDSIZE / 2 ][BOARDSIZE / 2 - 1] = BLACK;
    increment_cell_num();
    putp = allocput();
    putp->color = BLACK;
    putp->p.x = BOARDSIZE / 2;
    putp->p.y = BOARDSIZE / 2 - 1;
    append(&top, &(putp->main));
}


/*
 *" -b <n>           board size\n"
 *" -h <hostname>    hostname\n"
 *" -p <port>        port number\n"
 *" -l <level>       level\n"
 *" -m <mode>        mode: human/human(0, currently default)\n"
 *"                        human/computer(1),\n"
 *"                        computer/computer(2),\n"
 *"                        network(3)\n"
 *" -f <player>      first serve player (default: human)\n"
 *" -3               3D othello\n"
 *" -d <debug level> debug level\n"
 */

char *usage[] = {
    " Usage:\n"
    "  -b <n>           board size\n",
    "  -h <hostname>    hostname\n",
    "  -p <port>        port number\n",
    "  -l <level>       level\n",
    "  -m <mode>        mode: human/human(0, currently default)\n",
    "                         human/computer(1),\n",
    "                         computer/computer(2),\n",
    "                         network(3)\n",
    "  -f <player>      first serve player (default: human)\n",
    "  -3               3D othello\n",
    "  -d <debug level> debug level\n",
};


int opt_b;
int opt_h;
int opt_p;
int opt_l;
int opt_m;
int opt_f;
int opt_d;
int opt_3;

char remote_host[256];
unsigned short remote_port = 9360;
int level = 0;
int boardsize = BOARDSIZE;
int mode = MODE_HUMAN_HUMAN;
int serve_first = NO;
int debug_level = 0;

int option(int argc, char **argv) 
{
    int c, i;
    char *cp;

    opt_b = NO;
    opt_h = NO;
    opt_p = NO;
    opt_l = NO;
    opt_m = NO;
    opt_f = NO;
    opt_d = NO;
    opt_3 = NO;
    
    while (1) {
        c = getopt(argc, argv, "b:h:p:l:m:f:d:3");
        switch (c) {
        case 'b':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-b needs argument\n");
                    exit(255);
                }
                opt_b = YES;
                boardsize = atoi(optarg);
                if ((boardsize % 2) != 0) {
                    printf("board size must be even number\n");
                    exit(255);
                }
                if (boardsize > MAX_BOARDSIZE) {
                    printf("boardsize is restricted less than equal %d\n",
                           MAX_BOARDSIZE);
                    exit(255);
                }
                if (boardsize < MIN_BOARDSIZE) {
                    printf("boardsize is restricted more than equal %d\n",
                           MIN_BOARDSIZE);
                    exit(255);
                }
                        
            } else {
                dprintf("optstring is NULL");
                printf("-b needs argument\n");
                exit(255);
            }
            break;
            
        case 'h':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    dprintf("-h needs argument\n");
                    exit(255);
                }
                opt_h = YES;
                strcpy(remote_host, optarg);
            } else {
                dprintf("optstring is NULL");
                dprintf("-h needs argument\n");
                exit(255);
            }
            break;
            
        case 'p':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-p needs argument\n");
                    exit(255);
                }
                opt_p = YES;
                remote_port = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-p needs argument\n");
                exit(255);
            }
            break;
            
        case 'l':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-l needs argument\n");
                    exit(255);
                }
                opt_l = YES;
                level = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-p needs argument\n");
                exit(255);
            }
            break;
            
        case 'm':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-m needs argument\n");
                    exit(255);
                }
                opt_m = YES;
                mode = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-p needs argument\n");
                exit(255);
            }
            break;
            
        case 'f':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-f needs argument\n");
                    exit(255);
                }
                opt_f = YES;
                serve_first = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-f needs argument\n");
                exit(255);
            }
            break;
            
        case 'd':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-l needs argument\n");
                    exit(255);
                }
                opt_l = YES;
                level = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-d needs argument\n");
                exit(255);
            }
            break;
            
        case '3':
            dprintf("getopt returns %c\n", c);
            printf("Sorry. Option '-3' is not supported currently.\n");
            exit(255);
            break;
            
        case '?':
            printf("unknown parameter\n");
            for (i = 0; i < 11; i++) {
                printf(usage[i]);
            }
            
            exit(255);
            break;
        case -1:
            goto exit;
        }
        
    }
  exit:

    dprintf("Specified options summary\n");
    dprintf("  boardsize is %d\n", boardsize);
    dprintf("  remote_host is %s\n", remote_host);
    dprintf("  remote_port is %d\n", remote_port);
    dprintf("  level is %d\n", level);
    dprintf("  mode is %d\n", mode);
    dprintf("  serve_first is %d(0:HUMAN/1:COMPUTER))\n", serve_first);
    dprintf("  debug level is %d\n", debug_level);
    dprintf("  3D options is %d\n", opt_3);

    
}


void initialize(void)
{
    top.next = &top;
    top.prev = &top;

    candidate.next = &candidate;
    candidate.prev = &candidate;    
}




int main (int argc, char **argv)
{
    printf("othello %s   Written by M.Itoh (c)2002\n", VERSION);

    initialize();
    
    option(argc, argv);
    
    dprintf("initizlize board\n");
    initboard(&bd);

    dprintf("interactive\n");    
    interactive(&bd);
}

    
