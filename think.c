/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"


struct queue candidate;
void initput(struct put *p)
{
    if (p != NULL) {
        memset(p, 0x00, sizeof(struct put));
        INITQ(p->main);
        INITQ(p->candidate);
    }
    return;
}

int simple_search_candidate(struct board *bp, int color, struct queue *head)
{
    int x, y, pcount;
    struct put *putp;
    
    pcount = 0;
    putp = allocput();
    for (y = 0; y < BOARDSIZE; y++) {
        for (x = 0; x < BOARDSIZE; x++) {
            initput(putp);
            putp->color = color;
            putp->p.y = y;
            putp->p.x = x;
            if (check_puttable(bp, putp, color) == YES) {
                pcount++;
                tdprintf("think: found cell (%c%d), num=%d, gettable=%d\n",
                         (int)x + 'A' , y + 1, pcount, putp->gettable);
                append(head, &(putp->candidate));
                putp = allocput();
                    /*
                initput(putp);
                putp->color = color;
                putp->p.x = x;
                putp->p.x = y;
                    */
            }
        }
    }
    return pcount;
}

void cleanup_candidates(struct queue *canp)
{
    struct queue *qp, *qsavep;
    struct put *putp;
    
    qp = GET_TOP_ELEMENT(*canp);
    while (qp != canp) {
        struct queue *qsavep;
        qsavep = qp->next;
        dprintf("deleting qp=%p\n", qp);
        putp = (struct put *)((char *)qp -
                              offsetof(struct put, candidate));
        delete(&putp->candidate);
        freeput(putp);
        qp = qsavep;
    }
    return;
}

struct put *choose_max_gettable(struct queue *headp)
{
    struct queue *qp;
    struct put *putp, *gputp;
    
    gputp = NULL;
    if (IS_EMPTYQ(*headp)) {
        return gputp;
        
    } else {
        qp = GET_TOP_ELEMENT(*headp);
        gputp = CANDIDATE_TO_PUT(qp);
        while (qp != headp) {
            putp = CANDIDATE_TO_PUT(qp);
            if (gputp->gettable > putp->gettable) {
                gputp = putp;
            }
            qp = qp->next;
        }
    }
    return gputp;
}

int choose_corner_border(struct queue *candidatep,
                                   struct queue *cornerp,
                                   struct queue *borderp)
{
    int retcode = YES;
    struct queue *qp, *qpsave;
    struct put *putp, *gputp;
    
    qp = GET_TOP_ELEMENT(*candidatep);
/*    gputp = CANDIDATE_TO_PUT(qp); */
    gputp = NULL;
    while (qp != candidatep) {
        qpsave = qp->next;
        putp = CANDIDATE_TO_PUT(qp);
            /*
             * priority
             *  1) corner
             *  2) border
             *  3) maximun cells
             */
        if (((putp->p.x == 0) && (putp->p.y == 0)) ||
            ((putp->p.x == 0) && (putp->p.y == MAX_Y)) ||
            ((putp->p.x == MAX_X) && (putp->p.y == 0)) ||
            ((putp->p.x == MAX_X) && (putp->p.y == MAX_Y))) {
                /* corner */
            delete(&(putp->candidate));
            append(cornerp, &(putp->candidate));

        } else  if ((putp->p.x == 0) || (putp->p.x == MAX_X) ||
                    (putp->p.y == 0) || (putp->p.y == MAX_Y)) {
                /* border */
            delete(&(putp->candidate));
            append(borderp, &(putp->candidate));
            
        } else if (gputp == NULL) {
            gputp = putp;
            
        } else if ((gputp != NULL) && (putp->gettable > gputp->gettable)) {
            gputp = putp;
        }
        qp = qpsave;
    }
    if (gputp != NULL) {
        delete(&(gputp->candidate));
        push(&candidate, &(gputp->candidate));
    }
    
    return retcode;
}
#if 0
#define HORIZONTAL     1
#define VERTICAL       2
#define DIAGONAL       3
#define INV_DIAGONAL   4

int is_a_good_choice(struct board *bp, struct put *p, int direction)
{
    int retcode;
    int x, y;

    int before_myself = YES;

    int same_color_exist = NO;
    int opposite_color_exist = NO;
    
        /* for before side check */
    int first_stone_color = EMPTY;

    int dangerous = NO;
    
    
    if (IS_CORNER(p)) {
        return -1;
    }
    
    y = p->p.y;
    for (x = 0; x < bordsize; x++) {

        if (x == p->p.x) {
                /* check the before half */
            before_myself = NO;
            same_color_exist = NO;
            opposite_color_exist = NO;
            continue;
        }
        
        
        if (bp->x[x][y] == p->color) {
            /* SAME COLOR */
            if (before_myself == YES) {
                ;
                
            } else {
                ;
                
            }
            
        } else if (bp->[x][y] == OPPOSITE_COLOR(p->color)) {
            /* OPPOSITE COLOR */
            if (before_myself == YES) {
                ;
            } else {
                ;
            }
            
        } else {
            /* EMPTY */
            ;
            
        }

        /* check the after half */

        
        
#if 0            
        
        if (before_myself == YES) {
            if (bp->b[x][y] == p->color) {
                ;
            }
            
        } else {
            
        }
#endif        
    }
    
    
    ;
    return retcode;
}
#endif



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
int think_level1(struct board *bp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount;
    struct put *putp, *gputp;
    struct queue *qp;

    pcount = simple_search_candidate(bp, color, &candidate);
    tdprintf("think: found %d cells.\n", pcount);
    if (pcount > 0) {
        retcode = YES;
    } else {
        /*
         * PASS
         */
        return NO;
    }
    *p = *choose_max_gettable(&candidate);
    INITQ(p->candidate);  /* ugly */

    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    
    cleanup_candidates(&candidate);

    return retcode;
}

/*
 * (2) level 2 --- enhancement of (1) that is simple depth 1 search.
 *   search all from x=0,y=0, depth = 1, and
 *   choose the cell by which the number of cells I can get
 *   will be maximized, and if there is corner stone among possible cells,
 *   choose them in preference to ones that enables more cells.
 */
int think_level2(struct board *bp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount;
    struct put *putp, *gputp, *cgputp, *bgputp;
    struct queue *qp, corner, border;

    pcount = simple_search_candidate(bp, color, &candidate);
    tdprintf("think: found %d cells.\n", pcount);
    if (pcount > 0) {
        retcode = YES;
    } else {
        /*
         * PASS
         */
        return NO;
    }

    INITQ(corner);
    INITQ(border);
    
    choose_corner_border(&candidate, &corner, &border);
#if 1
    /* max gettable cell on neither corner nor border is the top of cand. */
    gputp = CANDIDATE_TO_PUT(GET_TOP_ELEMENT(candidate));
#else
    gputp = choose_max_gettable(&candidate);
#endif
    
    dprintf("corner\n");
    cgputp = choose_max_gettable(&corner);

    dprintf("border\n");
    bgputp = choose_max_gettable(&border);

    printf("%p,%p,%p\n", cgputp, bgputp, gputp);
    
    if (cgputp != NULL) {
        *p = *cgputp;
    
    } else if (bgputp != NULL) {
        *p = *bgputp;
        
    } else {
        *p = *gputp;
    }
    INITQ(p->candidate); /* ugly */
    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    
    cleanup_candidates(&candidate);
    cleanup_candidates(&corner);
    cleanup_candidates(&border);
    printf("think_level2 return\n");
    
    return retcode;
}

/*
 * (3) level 3 --- enhancement of (2) so that if the computer chooses
 *     a cell on border and it would be gotten by other player,
 *     avoid it. In case of no choice other than that,
 *     choose maximum gettable cell.
 */
int think_level3(struct board *bp, struct put *p, int color)
{
    int x, y, retcode, pcount;
    struct put *putp, *gputp, *cgputp, *bgputp;
    struct queue *qp, *qpsave, corner, border;

    pcount = simple_search_candidate(bp, color, &candidate);
    tdprintf("think: found %d cells.\n", pcount);
    if (pcount > 0) {
        retcode = YES;
    } else {
        /*
         * PASS
         */
        return NO;
    }

    INITQ(corner);
    INITQ(border);
    
    gputp = NULL;
    choose_corner_border(&candidate, &corner, &border);
    /* neigher corner nor border*/
    if (!IS_EMPTYQ(candidate)) {
        gputp = CANDIDATE_TO_PUT(GET_TOP_ELEMENT(candidate));
    }
    
    dprintf("corner\n");
    cgputp = choose_max_gettable(&corner);

    printf("border\n");
    bgputp = NULL;
    if (!IS_EMPTYQ(border)) {
        qp = GET_TOP_ELEMENT(border);
/*        bgputp = CANDIDATE_TO_PUT(qp);*/
        while (qp != &border) {
            putp = CANDIDATE_TO_PUT(qp);
            tdprintf("border check level=%d, %08x, (%d,%d)\n",
                   level, putp->neighbor.i, putp->p.x, putp->p.y);
            if ((putp->p.x == 0) || (putp->p.x == MAX_X)) {
                tdprintf("border check both sides x=%d\n", putp->p.x);
                    
                if (CHECK_NEIGHBOR(putp, UP) &&
                    CHECK_NEIGHBOR(putp, DOWN) &&
                    (bp->b[putp->p.x][putp->p.y - 1] ==
                     bp->b[putp->p.x][putp->p.y + 1])) {
                    tdprintf("Both of up and down exist.\n");
                        
                    bgputp = putp;
                        
                } else if (CHECK_NEIGHBOR(putp, UP)) {
                    if (bp->b[putp->p.x][putp->p.y - 1]
                            != OPPOSITE_COLOR(color)) {
                        tdprintf("choose border stone%d/%d\n",
                               putp->p.x, putp->p.y);
                        bgputp = putp;
                    }
                } else if (CHECK_NEIGHBOR(putp, DOWN)) {
                    if (bp->b[putp->p.x][putp->p.y + 1]
                        != OPPOSITE_COLOR(color)) {
                        tdprintf("choose border stone%d/%d\n",
                               putp->p.x, putp->p.y);
                        bgputp = putp;
                    }
                } else {
                    tdprintf("clean border cell %x/%x\n",putp->p.x, putp->p.y);
                    bgputp = putp;
                }
            }
            if ((putp->p.y == 0) || (putp->p.y == MAX_Y)) {
                tdprintf("hara\n");
                    
                if (CHECK_NEIGHBOR(putp, LEFT) &&
                    CHECK_NEIGHBOR(putp, RIGHT) &&
                    (bp->b[putp->p.x][putp->p.y - 1] ==
                     bp->b[putp->p.x][putp->p.y + 1])) {
                    tdprintf("hoge");
                    bgputp = putp;
                    
                } else if (CHECK_NEIGHBOR(putp, LEFT)) {
                    if (bp->b[putp->p.x - 1][putp->p.y]
                        != OPPOSITE_COLOR(color)) {
                        bgputp = putp;
                    }
                } else if (CHECK_NEIGHBOR(putp, RIGHT)) {
                    if (bp->b[putp->p.x + 1][putp->p.y]
                        != OPPOSITE_COLOR(color)) {
                        bgputp = putp;
                    }
                } else {
                    tdprintf("clean border cell %x/%x\n",putp->p.x, putp->p.y);
                    bgputp = putp;
                }
            }
            qp = qp->next;
        }
    }
    tdprintf("%p,%p,%p\n", cgputp, bgputp, gputp);
    
    if (cgputp != NULL) {
            /* If a puttalbe coner cell exists,
             cgputp MUST NOT be NULL. */
        *p = *cgputp;
    
    } else if (bgputp != NULL) {
        *p = *bgputp;
        
    } else if (gputp == NULL) {
        if (!IS_EMPTYQ(border)) {
            tdprintf("think_level3: It's rare!\n");
            *p = *choose_max_gettable(&border);
        } else {
            printf("think_level3: internal inconsistency\n");
            return NO;
        }
        
    } else {
        *p = *gputp;
    }
    
    INITQ(p->candidate);  /* ugly */
    
    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    
    cleanup_candidates(&candidate);
    cleanup_candidates(&corner);
    cleanup_candidates(&border);
    tdprintf("think_level3 return\n");
    
    return retcode;
}

int think_level4(struct board *bp, struct put *p, int color)
{
    int retcode = NO;
    
    return retcode;
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


