/*
 *   $Id$
 */
#ifndef _OTHELLO_H
#define _OTHELLO_H

#define VERSION "v0.2"

#define MAX_BOARDSIZE 8
#define MIN_BOARDSIZE 4
#define BOARDSIZE 8
#define MIN_X 0
#define MIN_Y 0
#define MAX_X (BOARDSIZE - 1)
#define MAX_Y (BOARDSIZE - 1)
#define MIN_PUTTABLE_OFFSET 2
#define PUT_CHECK_OFFSET 2

#define BLACK 1
#define WHITE -1
#define EMPTY 0

#define YES 1
#define NO  0

#define SERVED 1
#define PASS   2

#define MODE_HUMAN_HUMAN       0
#define MODE_HUMAN_COMPUTER    1
#define MODE_COMPUTER_COMPUTER 2
#define MODE_NETWORK           3

#define OPPOSITE_COLOR(color) ((color == BLACK) ? WHITE : BLACK)

#define INITQ(q) (q.next = &q, q.prev = &q)
#define IS_EMPTYQ(q) (q.next == &q)
#define GET_TOP_ELEMENT(queue) (queue).next
#define CANDIDATE_TO_PUT(queue) (struct put *)((char *)queue - \
        offsetof(struct put, candidate));




struct board 
{
    int b[BOARDSIZE][BOARDSIZE];
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

struct session
{
    struct queue main;         /* struct session main chain */
    struct board *bp;          /* */
};

struct scenario
{
    struct queue main;         /* struct scenario chain */
    struct queue candidate;    /* struct put chain */

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
extern void command_show(void);
/*
 * output.c
 */
extern int output(struct board *);
extern void initboard_format(void);
/*
 * util.c
 */
extern void append(struct queue *, struct queue *);
extern struct put *allocput(void);
void freeput(struct put *);
/*
 * put.c
 */
extern int check_empty(struct board *, struct put *, int);
extern int process_put(struct board *, struct put *, int);
extern int check_puttable(struct board *, struct put *, int);
/*
 * think.c
 */
extern int think(struct board *, struct put *, int);
extern int think_level0(struct board *, struct put *, int);
extern int think_level1(struct board *, struct put *, int);
extern int think_level2(struct board *, struct put *, int);
extern int think_level3(struct board *, struct put *, int);
extern int think_level4(struct board *, struct put *, int);

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




