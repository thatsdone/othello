/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "othello.h"

struct queue sessions;


int getpoint(struct session *sp, char *buf, struct point *p)
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
    
    if ((p->x >= sp->bd.xsize) || (p->y >= sp->bd.ysize) ||
        (p->x < 0) || (p->y < 0)) {
        return NO;
    }
    return YES;
}


int increment_cell_num(struct session *sp)
{
    sp->counter++;

    if (sp->counter == (sp->bd.xsize * sp->bd.ysize)) {
        return YES;
    } else {
        return NO;
    }
}

int get_occupied_cell_num(struct session *sp)
{
    return sp->counter;
}


void calculate_score(struct board *bp, int *black, int *white)
{
    int x, y;
    
    *black = 0;
    *white = 0;

    for (x = 0; x < bp->xsize; x++) {
        for (y = 0; y < bp->ysize; y++) {
            switch (CELL(*bp, x, y)) {
            case BLACK:
                (*black)++;
                break;
            case WHITE:
                (*white)++;
                break;
            }
        }
    }
    return;
}


void finalize(struct board *bp)
{
    int black, white;
    
    printf("Game Over!\n");
    calculate_score(bp, &black, &white);
    printf("BLACK: %d, WHITE: %d\n", black, white);
    if (black == white) {
        printf("Even result!\n");
        
    } else if (black > white) {
        printf("BLACK wins by %d stones\n", black - white);
        
    } else {
        printf("WHITE wins by %d stones\n", white - black);
    }

    return;
}

#define COMMAND_ILLEGAL   -1
#define COMMAND_INPUT      0
#define COMMAND_PASS       1
#define COMMAND_SHOW       2
#define COMMAND_SCORE      3
#define COMMAND_SAVE       4
#define COMMAND_LOAD       5
#define COMMAND_BACK       6
#define COMMAND_STATUS     7
#define COMMAND_HELP       8
#define COMMAND_BOARD      9
#define COMMAND_QUIT      10
#define COMMAND_TREE      11
#define COMMAND_DUMP      12
#define COMMAND_NULL      13

char *commands[] = {
    "  pass   : pass",
    "  show   : display internal control structures",
    "  score  : show score",
    "  save   : save this game into a transcript file",
    "  load   : load a game from a transcript file",
    "  back   : back",
    "  status : show system status",
    "  help   : show this message",
    "  board  : show current board",
    "  quite  : quit",    
    "*"
};


int getcommand(struct session *sp, struct put *putp)
{
    char buf[256], *bufp;

    memset(buf, 0x00, sizeof(buf));
    
    if (sp->is_end == YES) {
        printf("Command(GAME OVER): ");
    } else if (sp->turn == BLACK) {
        printf("Command(BLACK): ");
    } else {
        printf("Command(WHITE): ");
    }
    if (fgets(buf, 256, stdin) == NULL) {
        printf("null input\n");
        exit(255);
        
    }
    bufp = &(buf[0]);
        
    /* chop */
    buf[strlen(buf) - 1] = 0x00;
    dprintf("get '%s'\n", buf);
    
    if (strcmp(buf, "show") == 0) {
        return COMMAND_SHOW;
            
    } else if (strcmp(bufp, "score") == 0) {
        return COMMAND_SCORE;
           
    } else if (strcmp(bufp, "pass") == 0) {
        return COMMAND_PASS;

    } else if (strcmp(bufp, "help") == 0) {
        return COMMAND_HELP;
        
    } else if (strcmp(bufp, "board") == 0) {
        return COMMAND_BOARD;
        
    } else if (strcmp(bufp, "status") == 0) {
        return COMMAND_STATUS;
        
    } else if (strncmp(bufp, "save", 4) == 0) {
        sp->buf = malloc(strlen(bufp) + 1);
        strcpy(sp->buf, bufp);
        dprintf("input is %s\n", sp->buf);
        return COMMAND_SAVE;
        
    } else if (strcmp(bufp, "quit") == 0) {
        return COMMAND_QUIT;

    } else if (strcmp(bufp, "tree") == 0) {
        return COMMAND_TREE;

    } else if (strcmp(bufp, "dump") == 0) {
        return COMMAND_DUMP;

    } else if (strlen(bufp) == 0) {
        return COMMAND_NULL;
        
    } else {
            /*
             * normal input
             */
        if(getpoint(sp, bufp, &putp->p) != YES) {
            return COMMAND_ILLEGAL;
        }

        dprintf("(x,y)=(%d,%d)\n", putp->p.x, putp->p.y);
        
        return COMMAND_INPUT;
    }
}

char *usage[] = {
    " Usage:\n"
    "  -b <n>           board size\n",
    "  -h <hostname>    hostname\n",
    "  -p <port>        port number\n",
    "  -l <level>       level\n",
    "  -m <mode>        mode: human/human(0)\n",
    "                         human/computer(1) : default,\n",
    "                         computer/computer(2),\n",
    "                         network(3)\n",
    "  -f <player>      first serve player: 0=human(default)/1=computer\n",
    "  -3               3D othello\n",
    "  -d <debug level> debug level\n",
};

int serve_computer(struct session *sp, int player)
{
    int ret;
    struct put *putp;

    int savecolor;
    
    dprintf("serve_computer\n");
    
    putp = allocput();
    putp->color = sp->turn;

    savecolor = sp->turn;
    
    printf("Thinking...\n");
    ret = think(sp, putp, sp->turn);
    if (ret != YES) {
        printf("Pass!\n");
        putp->p.x = -1;
        putp->p.y = -1;        
        append(&(sp->top), &(putp->main));
        FLIP_COLOR(sp);
        if (sp->was_pass == YES) {
            sp->is_end = YES;
        }
        sp->was_pass = YES;
        return PASS;
    }
    sp->is_end = increment_cell_num(sp);
    append(&(sp->top), &(putp->main));
    ret = check_puttable(sp, &(sp->bd), putp, sp->turn);
    if (ret == YES) {
        CELL(sp->bd, putp->p.x, putp->p.y) = sp->turn;
    } else {
        printf("serve_computer: internal inconsistency: cannot put.(%c%d)\n",
               putp->p.x + 'A', putp->p.y + 1);
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
        exit(255);
    }
    dprintf("check_puttable returns %d\n", ret);
    ret = process_put(sp, &(sp->bd), putp, sp->turn);
    if (ret < 0) {
        printf("serve_computer: internal inconsistency: proces_put failed.\n");
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
        exit(255);
    }
    output(&(sp->bd));
    FLIP_COLOR(sp);
    return SERVED;
}



int serve_human(struct session *sp, int player)
{
    int ret;
    struct put *putp;

    dprintf("serve_human\n");
    
    putp = allocput();
    
    switch(getcommand(sp, putp)) {
    case COMMAND_ILLEGAL:
        output(&(sp->bd));
        printf("Illegal Input.\n");
        break;
            
    case COMMAND_INPUT:
        putp->color = sp->turn;
        ret = check_puttable(sp, &(sp->bd), putp, putp->color);
        if (ret == YES) {
            CELL(sp->bd, putp->p.x, putp->p.y) = putp->color;
            append(&(sp->top), &(putp->main));
            process_put(sp, &(sp->bd), putp, putp->color);
            output(&(sp->bd));            
            sp->is_end = increment_cell_num(sp);
            if (sp->is_end == YES) {
                finalize(&(sp->bd));
            }
            sp->was_pass = NO;
            FLIP_COLOR(sp);
        } else {
            output(&(sp->bd));
            printf("You cannot put there.\n");
        }
        
        break;
            
    case COMMAND_PASS:
        command_pass(sp, putp);
        break;
            
    case COMMAND_SHOW:
        command_show(sp);
        break;
            
    case COMMAND_SCORE:
    {
            
        int black, white;
        calculate_score(&(sp->bd), &black, &white);
        printf("BLACK:%d, WHITE:%d\n", black, white);
    }
        break;
            
    case COMMAND_SAVE:
        command_save(sp);
        break;
            
    case COMMAND_LOAD:
        break;
           
    case COMMAND_BACK:
        break;
            
    case COMMAND_STATUS:
        command_status(sp);
        break;
        
    case COMMAND_HELP:
    {
        int i = 0;
        while (commands[i][0] != '*') {
            printf("%s\n", commands[i]);
            i++;
        }
    }
        break;
        
    case COMMAND_BOARD:
        output(&(sp->bd));
        break;
        
    case COMMAND_QUIT:
        exit(0);
        break;
        
    case COMMAND_TREE:
        print_candidate_tree(
            &(sp->player[OPPOSITE_COLOR(sp->turn)].next_depth),
            1);;
        break;
        
    case COMMAND_DUMP:
        dump_data(sp);        
        break;
        
    case COMMAND_NULL:
        output(&(sp->bd));
        break;
        
    default:
	break;
    }
   
    return 0;
}


int serve_player(struct session *sp, int player)
{
    int ret;

    {
        int black, white;
        calculate_score(&(sp->bd), &black, &white);
        printf("\nCOUNT: %d, TURN: %d, BLACK:%d, WHITE:%d\n",
               get_occupied_cell_num(sp),
               sp->turn, black, white);
    }
    
    switch (sp->cfg.mode) {
    case MODE_HUMAN_HUMAN:
        ret = serve_human(sp, player);
        break;
        
    case MODE_HUMAN_COMPUTER:
    case MODE_NETWORK:
        if (sp->player[player].type == HUMAN) {
            ret = serve_human(sp, player);
        } else {
            ret = serve_computer(sp, player);
        }
        break;
        
    case MODE_COMPUTER_COMPUTER:
        ret = serve_computer(sp, player);
        break;
        
    default:
        printf("Internal inconsistency!");
        exit(255);
    }
    return ret;
}

void command_loop(struct session *sp)
{
    ;
    return;
}




void game(struct session *sp)
{
    int ret;
    int pass;

#define WHICH_TURN(sp) (sp)->turn
    if (sp->cfg.mode == MODE_HUMAN_COMPUTER)
        output(&(sp->bd));
    
    while (1) {
        switch(WHICH_TURN(sp)) {
        case PLAYER_FIRST:
            pass = serve_player(sp, PLAYER_FIRST);
            break;
            
        case PLAYER_SECOND:
            pass = serve_player(sp, PLAYER_SECOND);
            break;
            
        default:
            printf("Internal inconsistency!\n");
        }
        if (sp->is_end == YES) {
            finalize(&(sp->bd));
            while (1) {
                ret = serve_human(sp, BLACK);
            }
        }
    }
    
    return;
}


void option(int argc, char **argv, struct session *sp)
{
    int c, i;
    
    struct config *cfp;
    
    cfp = &(sp->cfg);
    
    cfp->level = 4;
    cfp->boardsize = BOARDSIZE;
    cfp->mode = MODE_HUMAN_COMPUTER;
    cfp->serve_first = NO;
    cfp->debug_level = 0;

    cfp->opt_b = NO;
    cfp->opt_h = NO;
    cfp->opt_p = NO;
    cfp->opt_l = NO;
    cfp->opt_m = NO;
    cfp->opt_f = NO;
    cfp->opt_d = NO;
    cfp->opt_0 = NO;
    cfp->opt_1 = NO;
    cfp->opt_3 = NO;
    cfp->opt_o = NO;
    cfp->opt_v = NO;
    cfp->opt_D = NO;
    sp->player[0].level = 4;
    sp->player[1].level = 4;    

    while (1) {
        c = getopt(argc, argv, "b:h:p:l:m:f:d:0:1:D:v3");
        switch (c) {
        case 'b':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-b needs argument\n");
                    exit(255);
                }
                cfp->opt_b = YES;
                cfp->boardsize = atoi(optarg);
                if ((cfp->boardsize % 2) != 0) {
                    printf("board size must be even number\n");
                    exit(255);
                }
                if (cfp->boardsize > MAX_BOARDSIZE) {
                    printf("boardsize is restricted less than equal %d\n",
                           MAX_BOARDSIZE);
                    exit(255);
                }
                if (cfp->boardsize < MIN_BOARDSIZE) {
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
                cfp->opt_h = YES;
                strcpy(cfp->remote_host, optarg);
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
                cfp->opt_p = YES;
                cfp->remote_port = atoi(optarg);
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
                cfp->opt_l = YES;
                cfp->level = atoi(optarg);
                printf("level is %d\n", cfp->level);
                
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
                cfp->opt_m = YES;
                cfp->mode = atoi(optarg);
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
                cfp->opt_f = YES;
                cfp->serve_first = atoi(optarg);
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
                cfp->opt_d = YES;
                cfp->debug_level = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-d needs argument\n");
                exit(255);
            }
            break;
            
        case '0':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-l needs argument\n");
                    exit(255);
                }
                cfp->opt_0 = YES;
                sp->player[BLACK].level = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-0 needs argument\n");
                exit(255);
            }
            break;
            
        case '1':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-l needs argument\n");
                    exit(255);
                }
                cfp->opt_1 = YES;
                sp->player[WHITE].level = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-1 needs argument\n");
                exit(255);
            }
            break;
            
        case 'D':
            dprintf("getopt returns %c\n", c);
            if (optarg != NULL) {
                dprintf("optstring is %s\n", optarg);
                if (optarg[0] == '-') {
                    printf("-l needs argument\n");
                    exit(255);
                }
                cfp->opt_D = YES;
                cfp->depth = atoi(optarg);
            } else {
                dprintf("optstring is NULL");
                printf("-1 needs argument\n");
                exit(255);
            }
            break;
            
        case '3':
            dprintf("getopt returns %c\n", c);
            printf("Sorry. Option '-3' is not supported currently.\n");
            exit(255);
            break;
            
        case 'v':
            for (i = 0; i < 11; i++) {
                printf(usage[i]);
            }
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
    
    if ((cfp->opt_l == YES) && ((cfp->opt_0 == YES) || (cfp->opt_1 == YES))) {
        printf("Option -l and -0, -1 are mutually exclusive.\n");
        exit(255);
    }
    
    if ((cfp->level == 4) && (cfp->opt_D == NO)) {
        cfp->depth = 2;
        printf("Level %d Default depth %d used.\n",
               cfp->level, cfp->depth);
    }
    
    dprintf("Specified options summary\n");
    dprintf("  boardsize is %d\n", cfp->boardsize);
    dprintf("  remote_host is %s\n", cfp->remote_host);
    dprintf("  remote_port is %d\n", cfp->remote_port);
    dprintf("  level is %d\n", cfp->level);
    dprintf("  mode is %d\n", cfp->mode);
    dprintf("  serve_first is %d(0:HUMAN/1:COMPUTER))\n", cfp->serve_first);
    dprintf("  debug level is %d\n", cfp->debug_level);
    dprintf("  3D options is %d\n", cfp->opt_3);
     printf("  depth is %d\n", cfp->depth);    
    
    return;
}

int initialize_network(void)
{
    return NO;
}


int initboard(struct session *sp)
{
    int x, y, boardsize;
    struct put *putp;

    boardsize = sp->cfg.boardsize;
    
    sp->bd.b = (int *)malloc(sizeof(int) *
                              sp->cfg.boardsize * sp->cfg.boardsize);
    sp->bd.xsize = boardsize;
    sp->bd.ysize = boardsize;

    for (x = 0; x < boardsize; x++) {
        for (y = 0; y < boardsize; y++) {
            CELL(sp->bd, x, y) = EMPTY;
            dprintf("(%d,%d) is %d\n", x, y, CELL(sp->bd, x,  y));
            
        }
    }
    printf("boardsize = %d\n", boardsize);
    
    CELL(sp->bd, (boardsize / 2 - 1), (boardsize / 2)) = BLACK;
    increment_cell_num(sp);
    putp = allocput();
    putp->color = BLACK;
    putp->p.x = boardsize / 2 - 1;
    putp->p.y = boardsize / 2;
    append(&(sp->top), &(putp->main));
    
    CELL(sp->bd, (boardsize / 2 - 1), (boardsize / 2 - 1)) = WHITE;
    increment_cell_num(sp);
    putp = allocput();
    putp->color = WHITE;
    putp->p.x = boardsize / 2 - 1;
    putp->p.y = boardsize / 2 - 1;
    append(&(sp->top), &(putp->main));
    
    CELL(sp->bd, boardsize / 2 , boardsize / 2 - 1) = BLACK;
    increment_cell_num(sp);
    putp = allocput();
    putp->color = BLACK;
    putp->p.x = boardsize / 2;
    putp->p.y = boardsize / 2 - 1;
    append(&(sp->top), &(putp->main));
    
    CELL(sp->bd, boardsize / 2, boardsize / 2) = WHITE;
    increment_cell_num(sp);
    putp = allocput();
    putp->color = WHITE;
    putp->p.x = boardsize / 2;
    putp->p.y = boardsize / 2;
    append(&(sp->top), &(putp->main));

}


struct session *initialize(int argc, char **argv)
{
    struct session *sp;
    struct depth *dp;

    sp = (struct session *)malloc(sizeof (struct session));
    memset(sp, 0x00, sizeof(struct session));
    INITQ(sp->main);
    dprintf("option\n");
    option(argc, argv, sp);
    if (sp->cfg.mode == MODE_NETWORK) {
        if (initialize_network() != YES) {
            printf("Network initialization error!\n");
            exit(255);
        }
    }
    
    /* initialize this session's board */
    dprintf("board\n");
    INITQ(sp->top);
    sp->counter  = 0;
    initboard(sp);
    /* initialize player */
    dprintf("player\n");
    switch (sp->cfg.mode) {
    case MODE_HUMAN_HUMAN:
        sp->player[PLAYER_FIRST].type = HUMAN;
        sp->player[PLAYER_SECOND].type = HUMAN;
        break;
        
    case MODE_HUMAN_COMPUTER:
    case MODE_NETWORK:
        if (sp->cfg.serve_first == NO) {
            sp->player[PLAYER_FIRST].type   = HUMAN;
            sp->player[PLAYER_FIRST].level  = -1;
            sp->player[PLAYER_SECOND].type  = COMPUTER;
            sp->player[PLAYER_SECOND].level = sp->cfg.level;
            
        } else {
            sp->player[PLAYER_FIRST].type   = COMPUTER;
            sp->player[PLAYER_FIRST].level  = sp->cfg.level;
            sp->player[PLAYER_SECOND].type  = HUMAN;
            sp->player[PLAYER_SECOND].level = -1;
        }
        break;
        
    case MODE_COMPUTER_COMPUTER:
        sp->player[PLAYER_FIRST].type   = COMPUTER;
        sp->player[PLAYER_SECOND].type  = COMPUTER;
        break;
        
    default:
	break;
            /* never */
    }
    INITQ(sp->player[PLAYER_FIRST].candidate);
    INITQ(sp->player[PLAYER_SECOND].candidate);
    INITQ(sp->player[PLAYER_FIRST].next_depth);
    INITQ(sp->player[PLAYER_SECOND].next_depth);
    INITQ(sp->player[PLAYER_FIRST].depth);
    INITQ(sp->player[PLAYER_SECOND].depth);
    sp->player[PLAYER_FIRST].num_candidate   = 0;
    sp->player[PLAYER_SECOND].num_candidate  = 0;
    sp->turn     = BLACK;
    sp->was_pass = NO;
    sp->is_end   = NO;

    append(&(sessions), &(sp->main));
    
    {
        time_t t;
        int i;
        time(&t);
        srandom((unsigned int)t);
    }
    
    return sp;
    
}




int main (int argc, char **argv)
{
    struct session *sessionp;
    
    printf("othello %s   Written by M.Itoh (c)2002\n", VERSION);
    INITQ(sessions);
    sessionp = initialize(argc, argv);
    dprintf("level=%d/%d\n",
            sessionp->player[0].level,
            sessionp->player[1].level);
    dprintf("mode=%d\n", sessionp->cfg.mode);
    dprintf("size=%d\n", sessionp->cfg.boardsize);
    dprintf("serve_first=%d\n", sessionp->cfg.serve_first);    

    game(sessionp);
    

}

    


