/*
 *   $Id$
 */
#ifndef _OTHELLO_H
#define _OTHELLO_H

#define VERSION "v0.2"

#define MAX_BOARDSIZE 16
#define MIN_BOARDSIZE 4
#define BOARDSIZE 8
#define MIN_X 0
#define MIN_Y 0
#define MAX_X(sp) ((sp)->bd.xsize - 1)
#define MAX_Y(sp) ((sp)->bd.ysize - 1)
#define MIN_PUTTABLE_OFFSET 2
#define PUT_CHECK_OFFSET 2

#define BLACK 0
#define WHITE 1
#define EMPTY -1

#define YES 1
#define NO  0

#define SERVED 1
#define PASS   2

#define MODE_HUMAN_HUMAN       0
#define MODE_HUMAN_COMPUTER    1
#define MODE_COMPUTER_COMPUTER 2
#define MODE_NETWORK           3

#define OPPOSITE_COLOR(color) ((color == BLACK) ? WHITE : BLACK)

#define FLIP_COLOR(sp) sp->turn = OPPOSITE_COLOR(sp->turn)

#define INITQ(q) ((q).next = &(q), (q).prev = &(q))
#define IS_EMPTYQ(q) ((q).next == &(q))
#define IS_ENDQ(wqp, topq) ((wqp) == &(topq))

#define GET_TOP_ELEMENT(queue) (queue).next
#define CANDIDATE_TO_PUT(queue) (struct put *)((char *)queue - \
        offsetof(struct put, candidate))
#define MAIN_TO_PUT(q) (struct *put)(q)

#define SET_NEIGHBOR(putp, direction) putp->neighbor.i |= (1UL << direction)
#define CHECK_NEIGHBOR(putp, direction) putp->neighbor.i & (1UL << direction)

#define SET_CANGET(putp, direction) putp->canget.i |= (1UL << direction)
#define CHECK_CANGET(putp, direction) putp->canget.i & (1UL << direction)

#define IS_PUTTABLE_RANGE(x, y, bp) ((x >= MIN_PUTTABLE_OFFSET) && \
                                   (y >= MIN_PUTTABLE_OFFSET) && \
                                   (x <= bp->xsize - MIN_PUTTABLE_OFFSET ) && \
                                   (y <= bp->ysize - MIN_PUTTABLE_OFFSET))

#define IS_SCANNABLE_RANGE(x, y, bp) ((x >= 0) && \
                                   (y >= 0) && \
                                   (x < bp->xsize) && \
                                   (y < bp->ysize))

#define CELL(bd, x, y) *(((bd).b) + (x) + (y) * (bd).ysize)
#define IS_EMPTY_CELL(bd, x, y) (CELL(bd, x, y) == EMPTY)


struct board 
{
    int *b;
    int xsize;
    int ysize;
};


struct point 
{
    int x;
    int y;
};

#define UP             0
#define DOWN           1
#define LEFT           2
#define RIGHT          3
#define UPPER_LEFT     4
#define UPPER_RIGHT    5
#define LOWER_LEFT     6
#define LOWER_RIGHT    7

#define NUM_DIRECTION 8

struct queue 
{
    struct queue *next;
    struct queue *prev;
};

struct direction 
{
    int up:1;
    int down:1;
    int left:1;
    int right:1;
    int upper_left:1;
    int upper_right:1;
    int lower_left:1; 
    int lower_right:1;
    int reserve:24;
};


struct put
{
    struct queue main;
    struct queue candidate;
    int color;
    struct point p;
    
    union {
        struct direction b;
        int i;
    } neighbor;
    union {
        struct direction b;
        int i;
    } canget;
    int gettable;

};

struct config
{
    int boardsize;
    int level;
    int mode;
    int serve_first;
    int debug_level;
    char remote_host[256];
    unsigned short remote_port;
    int opt_b;
    int opt_h;
    int opt_p;
    int opt_l;
    int opt_m;
    int opt_f;
    int opt_d;
    int opt_3;
};
#if 0
struct scenario
{
    struct queue main;         /* struct scenario chain */
    struct queue candidate;    /* struct put chain */
};
#endif

#define HUMAN    0
#define COMPUTER 1

struct player
{
    int type;                           /* player type HUMAN/COMPUTER */
    int level;
    struct queue candidate;
};

#define NUM_PLAYER 2
#define PLAYER_FIRST  0
#define PLAYER_SECOND 1

struct session
{
    struct queue main;                  /* struct session main chain */
    struct config cfg;                  /* struct config */
    struct board bd;                    /* struct board */
    struct queue top;                   /* transcript queue */
    struct player player[NUM_PLAYER];   /* */
    int turn;
    int was_pass;
    int is_end;
};


#ifdef DEBUG1
#define dprintf printf
#else
#define dprintf
#endif

#ifdef DEBUG
#define tdprintf printf
#else
#define tdprintf
#endif


/*
 * command.c
 */
extern void command_show(struct session *);
/*
 * output.c
 */
extern int output(struct board *);
extern void initboard_format(void);
/*
 * util.c
 */
extern void append(struct queue *, struct queue *);
void push(struct queue *, struct queue *);
void delete(struct queue *);
extern struct put *allocput(void);
void freeput(struct put *);
/*
 * put.c
 */
extern int check_empty(struct session *, struct put *, int);
extern int process_put(struct session *, struct put *, int);
extern int check_puttable(struct session *, struct put *, int);
/*
 * think.c
 */
extern int think(struct session *, struct put *, int);
extern int think_level0(struct session *, struct put *, int);
extern int think_level1(struct session *, struct put *, int);
extern int think_level2(struct session *, struct put *, int);
extern int think_level3(struct session *, struct put *, int);
extern int think_level4(struct session *, struct put *, int);

/*
 * othello.c
 */
extern char remote_host[256];
extern unsigned short remote_port;
extern int level;
extern int boardsize;
extern int mode;
extern int serve_first;
extern int debug_level;
extern struct queue candidate;

#endif /* _OTHELLO_H */








