/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"


struct queue candidate;

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
     *   choose first gettable point :(
     */
    case 1:
        pcount = 0;
        putp = allocput();
        for (y = 0; y < BOARDSIZE; y++) {
            putp->color = color;
            putp->p.y = y;
            for (x = 0; x < BOARDSIZE; x++) {
                putp->p.x = x;
/*                printf("x=%d, y=%d\n", x,y);*/
                if (check_puttable(bp, putp, color) == YES) {
                    pcount++;
                    tdprintf("think: found puttable place %d/%d, num=%d, gettable=%d\n",
                             x, y, pcount, putp->gettable);
                    append(&candidate, &(putp->candidate));
                    putp = allocput();
                    retcode = YES;
                }
            }
        }
        tdprintf("think: found %d puttable point.\n", pcount);
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
        printf("use x=%d,y=%d,gettable=%d\n", p->p.x, p->p.y, p->gettable);
        
        qp = candidate.next;
        while (qp != &candidate) {
            struct queue *qsavep;
            qsavep = qp->next;
            printf("deleting qp=%p\n", qp);
            putp = (struct put *)((char *)qp -
                                  offsetof(struct put, candidate));
            delete(&putp->candidate);
            free((void *)putp);
            qp = qsavep;
        }
        break;
        
    default:
        printf("level=%d is not implemented yet.\n", level);
        exit(255);
    }
    return retcode;
}


