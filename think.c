/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"


struct queue candidate;

int think_level0(struct board *bp, struct put *p, int color)
{
    return YES;
}

int think_level1(struct board *bp, struct put *p, int color)
{
    return YES;
}

int think_level2(struct board *bp, struct put *p, int color)
{
    return YES;
}

int think_level3(struct board *bp, struct put *p, int color)
{
    return YES;
}

int think_level4(struct board *bp, struct put *p, int color)
{
    return YES;
}


int think(struct board *bp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount, gcount;
    struct put *putp, *gputp;
    struct queue *qp;
    
#if 0
    p->color = color;
    if (serve_first == NO) {
        p->p.x = 4;
        p->p.y = 5;
    } else {
        p->p.x = 3;
        p->p.y = 5;
    }
#endif

    switch (level) {
    /*
     * (0) level 0 --- no-brain approach
     *   search all from x=0,y=0, depth = 1,
     *   choose first gettable point :(
     */
    case 0:
        putp = allocput();
        for (y = 0; y < BOARDSIZE; y++) {
            putp->p.y = y;
            for (x = 0; x < BOARDSIZE; x++) {
                putp->p.x = x;
                if (check_puttable(bp, putp, color) == YES) {
                    tdprintf("think: found puttable place x=%d/y=%d,gettable=%d\n", x, y, putp->gettable);
                    *p = *putp;
                    retcode = YES;
                    goto exit;
                }
            }
        }
      exit:
        break;
        
    /*
     * (1) level 1 --- no-brain approach part II
     *   search all from x=0,y=0, depth = 1,
     *   choose the cell by which the number of cells I can get
     *   will be maximized.
     */
    case 1:
        pcount = 0;
        putp = allocput();
        for (y = 0; y < BOARDSIZE; y++) {
            for (x = 0; x < BOARDSIZE; x++) {
                putp->color = color;
                putp->p.y = y;
                putp->p.x = x;
/*                printf("x=%d, y=%d\n", x,y);*/
                if (check_puttable(bp, putp, color) == YES) {
                    pcount++;
                    tdprintf("think: found puttable place (%c%d), num=%d, gettable=%d\n",
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
        tdprintf("think: found %d puttable point.\n", pcount);
        /* PASS */
        if (pcount == 0) {
            freeput(putp);
            retcode = NO;
            break;
        }
        qp = candidate.next;
        gputp = (struct put *)((char *)qp -
                                  offsetof(struct put, candidate));
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
        break;

    case 2:
        
    default:
        printf("level=%d is not implemented yet.\n", level);
        exit(255);
    }
    return retcode;
}


