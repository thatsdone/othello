/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"


struct queue candidate;

/*
 * (0) level 0 --- no-brain approach
 *   search all from x=0,y=0, depth = 1,
 *   choose first gettable point :(
 */
int think_level0(struct board *bp, struct put *p, int color)
{
    int x, y, retcode = NO;
    struct put *putp;

    putp = allocput();
    for (y = 0; y < BOARDSIZE; y++) {
        putp->p.y = y;
        for (x = 0; x < BOARDSIZE; x++) {
            putp->p.x = x;
            if (check_puttable(bp, putp, color) == YES) {
                tdprintf("think: found puttable place x=%d/y=%d,gettable=%d\n",
                         x, y, putp->gettable);
                *p = *putp;
                goto exit;
            }
        }
    }
  exit:
    freeput(putp);
    return retcode;
}

/*
 * (1) level 1 --- no-brain approach part II
 *   search all from x=0,y=0, depth = 1,
 *   choose the cell by which the number of cells I can get
 *   will be maximized.
 */
int simple_search_puttable(struct board *bp, int color, struct queue *head)
{
    int x, y, pcount;
    struct put *putp;
    
    pcount = 0;
    putp = allocput();
    for (y = 0; y < BOARDSIZE; y++) {
        for (x = 0; x < BOARDSIZE; x++) {
            putp->color = color;
            putp->p.y = y;
            putp->p.x = x;
            if (check_puttable(bp, putp, color) == YES) {
                pcount++;
                tdprintf("think: found cell (%c%d), num=%d, gettable=%d\n",
                         (int)x + 'A' , y + 1, pcount, putp->gettable);
                append(head, &(putp->candidate));
                putp = allocput();
                putp->color = color;
                putp->p.x = x;
                putp->p.x = y;                    
            }
        }
    }
    return pcount;
}



int think_level1(struct board *bp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount, gcount;
    struct put *putp, *gputp;
    struct queue *qp;

#if 0    
    pcount = 0;
    putp = allocput();
    for (y = 0; y < BOARDSIZE; y++) {
        for (x = 0; x < BOARDSIZE; x++) {
            putp->color = color;
            putp->p.y = y;
            putp->p.x = x;
            if (check_puttable(bp, putp, color) == YES) {
                pcount++;
                tdprintf("think: found cell (%c%d), num=%d, gettable=%d\n",
                         (int)x + 'A' , y + 1, pcount, putp->gettable);
                append(&candidate, &(putp->candidate));
                putp = allocput();
                putp->color = color;
                putp->p.x = x;
                putp->p.x = y;                    
                retcode = YES;
            }
        }
    }
#else
    pcount = simple_search_puttable(bp, color, &candidate);
    if (pcount > 0) retcode = YES;
    
#endif    
    tdprintf("think: found %d cells.\n", pcount);
    if (pcount == 0) {
        /* PASS */
#if 0
        freeput(putp);
#endif
        /* retcode = NO;*/
        return NO;
    }
#define GET_MAIN(queue) queue.next
#if 0    
    qp = candidate.next;
#else
    qp = GET_MAIN(candidate);
#endif
#if 0    
    gputp = (struct put *)((char *)qp -
                                  offsetof(struct put, candidate));
#else
#define CANDIDATE_TO_PUT(queue) (struct put *)((char *)queue - \
        offsetof(struct put, candidate));
    gputp = CANDIDATE_TO_PUT(qp);
#endif
    while (qp != &candidate) {
        dprintf("checking qp=%p\n", qp);
        
        putp = (struct put *)((char *)qp -
                              offsetof(struct put, candidate));
        if (putp->gettable > gputp->gettable) {
            gputp = putp;
        }
        qp = qp->next;
    }
    *p = *gputp;
    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
        
    qp = candidate.next;
    while (qp != &candidate) {
        struct queue *qsavep;
        qsavep = qp->next;
        dprintf("deleting qp=%p\n", qp);
        putp = (struct put *)((char *)qp -
                              offsetof(struct put, candidate));
        delete(&putp->candidate);
        freeput(putp);
        qp = qsavep;
    }
    return retcode;
}

/*
 * (2) level 2 --- enhancement of (1) that is simple depth 1 search.
 *   search all from x=0,y=0, depth = 1,
 *   choose the cell by which the number of cells I can get
 *   will be maximized, and if there is corner stone among possible cells,
 *   choose them in preference to ones that enables more cells.
 */
int think_level2(struct board *bp, struct put *p, int color)
{
    return NO;
}

int think_level3(struct board *bp, struct put *p, int color)
{
    return NO;
}

int think_level4(struct board *bp, struct put *p, int color)
{
    return NO;
}


int think(struct board *bp, struct put *p, int color)
{
    int retcode;
    
    switch (level) {
    case 0:
        retcode = think_level0(bp, p, color);
        break;

    case 1:
        retcode = think_level1(bp, p, color);
        break;

    case 2:
        retcode = think_level2(bp, p, color);
        break;

    case 3:
        retcode = think_level3(bp, p, color);
        break;

    case 4:
        retcode = think_level4(bp, p, color);
        break;

    default:
        printf("level=%d is not implemented yet.\n", level);
        exit(255);
    }
    return retcode;
}


