/*
 *   $Id$
 */
#ifndef _OTHELLO_H
#define _OTHELLO_H

#include <stddef.h>

#define VERSION "v0.7"

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

#define SUCCESS 1
#define FAIL    -1

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
#define GET_LAST_ELEMENT(queue) (queue).prev
#define CANDIDATE_TO_PUT(queue) ((struct put *)((char *)queue - \
        offsetof(struct put, candidate)))
#define MAIN_TO_PUT(q) (struct put *)(q)
#define DEPTH_TO_PUT(queue) (struct put *)((char *)queue - \
        offsetof(struct put, depth))

#define NEXTDEPTH_TO_PUT(q) (struct put *)((char *)q - \
        offsetof(struct put, next_depth))

#define Q_TO_DEPTH(queue) (struct depth *)((char *)queue - \
        offsetof(struct depth, q))

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
#define SET_CELL(bd, x, y, color) CELL(bd, x, y) = (color)

#define PLAYER_TYPE(type) ((type == HUMAN) ? "HUMAN" : "COMPUTER")

#define IS_CORNER(sp, putp) ((putp->p.x == 0) && (putp->p.y == 0)) || \
                       ((putp->p.x == 0) && (putp->p.y == MAX_Y(sp))) || \
                       ((putp->p.x == MAX_X(sp)) && (putp->p.y == 0)) || \
                       ((putp->p.x == MAX_X(sp)) && (putp->p.y == MAX_Y(sp)))

#define IS_BORDER(sp, putp) ((putp->p.x == 0) || (putp->p.x == MAX_X(sp)) || \
                             (putp->p.y == 0) || (putp->p.y == MAX_Y(sp)))


#define IS_CENTER(sp, putp) (putp->p.x >= MIN_PUTTABLE_OFFSET) && \
                            (putp->p.x <= sp->bd.xsize - \
                             MIN_PUTTABLE_OFFSET) && \
                            (putp->p.y >= MIN_PUTTABLE_OFFSET) && \
                            (putp->p.y <= sp->bd.ysize - \
                             MIN_PUTTABLE_OFFSET)

#define IS_SUB_BORDER(sp, putp) (putp->p.x == 1) || \
                                (putp->p.x == MAX_X(p) - 1) || \
                                (putp->p.y == 1) || \
                                (putp->p.y == MAX_Y(p) - 1)

#define IS_SAME_CELL(putp1, putp2) ((putp1->p.x == putp2->p.x) && \
                                    (putp1->p.y == putp2->p.y))


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
    struct queue main;             /* main transcript queue */
    struct queue candidate;        /* candidate queue */
    int color;                     /* player (color )*/
    struct point p;                /* cell cordinate */
    /*
     * for puttable checking bitmap
     */
    union {
        struct direction b;
        int i;
    } neighbor;
    union {
        struct direction b;
        int i;
    } canget;
    int gettable;
        /*
         * only for level 4
         */
    struct queue next_depth;       /* QH for next depth candidates from this */
    struct queue depth;            /* QE for the same parent candidate chain */
    struct board *bp;              /* board with this put */
    struct put *up;                /* upper depth parent candidate */
    struct depth *dp;              /* ptr for struct depth of this level */
    int black;
    int white;
};

struct config
{
        /* command line option data */
    int boardsize;               /* -b: board size, assuming square lattice */
    int level;
    int mode;
    int serve_first;
    int debug_level;
    char remote_host[256];
    unsigned short remote_port;
    int depth; /* tmp */
    int opt_b;
    int opt_h;
    int opt_p;
    int opt_l;
    int opt_m;
    int opt_f;
    int opt_d;
    int opt_0;
    int opt_1;
    int opt_3;
    int opt_o;
    int opt_v;
    int opt_D;
};

#define HUMAN    0
#define COMPUTER 1

struct depth
{
    struct queue q;                     /* struct depth chain head */
    struct queue candidate;             /* candidate struct put chain head */
#if 0
    int depth;
    struct queue next_depth;            /* only for depth=1 */
#endif
    int num_cand;
};

struct player
{
    int type;                           /* player type HUMAN/COMPUTER */
    int level;
    struct queue candidate;             /* QH for struct put chain lv < 3 */
    struct queue next_depth;            /* QH for depth=1 put->depth */
    struct queue depth;                 /* QH for struct depth chain */
    int num_candidate;
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
    int counter;
    int turn;
    int was_pass;
    int is_end;
    void *buf;
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
extern int  command_status(struct session *);
extern int  command_pass(struct session *, struct put *);
extern int  command_save(struct session *);

/*
 * output.c
 */
extern int output(struct board *);
extern void initboard_format(void);
/*
 * util.c
 */
extern void append(struct queue *, struct queue *);
extern void push(struct queue *, struct queue *);
extern void delete(struct queue *);
extern void requeue_all(struct queue *, struct queue *);
extern struct put *allocput(void);
extern void freeput(struct put *);
extern void initput(struct put *);
extern void init_depth(struct depth *, int);
extern struct depth *alloc_depth(void);
extern int num_put;
extern int orand(int);
extern struct board *dup_board(struct session *, struct board *);
extern void freeboard(struct board *);
extern void cleanup_boards(struct depth *);

/*
 * put.c
 */
extern int check_empty(struct session *, struct board *, struct put *, int);
extern int process_put(struct session *, struct board *, struct put *, int);
extern int check_puttable(struct session *, struct board *, struct put *, int);

/*
 * think.c
 */
extern int think(struct session *, struct put *, int);
extern int think_level0(struct session *, struct put *, int);
extern int think_level1(struct session *, struct put *, int);
extern int think_level2(struct session *, struct put *, int);
extern int think_level3(struct session *, struct put *, int);
extern int think_level4(struct session *, struct put *, int);
extern void print_candidate_tree(struct queue *, int);

/*
 * othello.c
 */
extern struct depth *allocdepth(void);
extern void initdepth(struct depth *, int);
void freedepth(struct depth *);


#define STRATEGY_RANDOM           0x0001
#define STRATEGY_MINI_MAX         0x0002
#define STRATEGY_MINI_MIN         0x0004
#define STRATEGY_CENTER_ORIENTED  0x0008
#define STRATEGY_LIMB_ORIENTED    0x0010

#define STRATEGY_CORNER_BORDER    0x0020

#define STRATEGY_GA               100
#define STRATEGY_NEURALNET        101

extern struct put *simple_strategy(struct session *, unsigned int);
extern int search_depth(struct session *, int);



#endif /* _OTHELLO_H */

