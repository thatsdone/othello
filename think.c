/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"


struct board *dup_board(struct session *sp, struct board *origp)
{
    struct board *bp;
    size_t size;

    size = sizeof(int) * origp->xsize * origp->ysize;
    bp = (struct board *)malloc(sizeof(struct board));
    if (bp == NULL) {
        printf("dup_board: malloc failed(1)\n");
        return NULL;
    }
    bp->b = (int *)malloc(size);
    if (bp->b == NULL) {
        free(bp);
        printf("dup_board: malloc failed(2)\n");
        return NULL;
    }
    bp->xsize = origp->xsize;
    bp->ysize = origp->ysize;
    memcpy(bp->b, origp->b, size);
    
    return bp;
}


#define SINGLE_DEPTH 0
#define MULTIPLE_DEPTH 1

int simple_search_candidate(struct session *sp, int color, struct queue *head)
{
    struct board *bp = &(sp->bd);
    return simple_search_candidate_mp(sp, bp, color, head, SINGLE_DEPTH, NULL);
}

int simple_search_candidate_mp(struct session *sp,
                               struct board *bp,
                               int color,
                               struct queue *candidatep,
                               int mode,
                               struct queue *next_depthp)
{
    int x, y, pcount;
    struct put *putp;
    
    pcount = 0;
    putp = allocput();
    for (y = 0; y < sp->bd.xsize; y++) {
        for (x = 0; x < sp->bd.ysize; x++) {
            initput(putp);
            putp->color = color;
            putp->p.y = y;
            putp->p.x = x;
            if (check_puttable(sp, bp, putp, color) == YES) {
                pcount++;
                dprintf("simple_search_candidate_mp: found cell (%c%d), num=%d, gettable=%d, %p\n",
                         (int)x + 'A' , y + 1, pcount, putp->gettable, putp);
                if (mode == SINGLE_DEPTH) {
                    append(candidatep, &(putp->candidate));
                } else {
                    append(candidatep, &(putp->candidate));
                    append(next_depthp, &(putp->depth));

                    putp->bp = dup_board(sp, bp);
#if 0
                    if ((putp->p.x == 2) && (putp->p.y == 2)) {
                        printf("putp->bp=%p, bp=%p\n", putp->bp, bp);
            
                        output(putp->bp);
                    }
#endif
                    SET_CELL(*(putp->bp), putp->p.x, putp->p.y, putp->color);
                    process_put_bd(sp, putp->bp, putp, putp->color);
                    dprintf("simple_search_candidate_mp: %p\n", putp->bp);
#if 0
                    if ((putp->p.x == 2) && (putp->p.y == 2)) {
                        printf("hagehagehage:\n putp->bp=%p\n", putp->bp);
                        output(putp->bp);
                        printf("bp=%p\n", bp);
                        output(bp);
                    }
#endif
                }
                putp = allocput();
            }
        }
    }
#if 0
    if (pcount == 0) {
        for(x = 0; x < 8; x++) {
            for(y = 0; y < 8; y++) {
                printf("CELL (x,y)=(%d,%d) is %d(%08x/%08x)\n",
                       x, y, CELL(sp->bd, x, y),
                       putp->neighbor.i, putp->canget.i);
            }
        }
    }
#endif
    if (pcount == 0) {
        freeput(putp);
    }
    
        
    return pcount;
}


void cleanup_board(struct board *bp)
{
    free(bp->b);
    free(bp);
}


void cleanup_candidates(struct queue *canp)
{
    struct queue *qp, *qsavep;
    struct put *putp;
    
    qp = GET_TOP_ELEMENT(*canp);
    while (qp != canp) {
        qsavep = qp->next;
        printf("deleting qp=%p\n", qp);
        putp = CANDIDATE_TO_PUT(qp);
        delete(&putp->candidate);
/*        delete(&putp->depth);         */
        if (putp->bp != NULL) {
            cleanup_board(putp->bp);
        }
        freeput(putp);
        qp = qsavep;
    }
    return;
}



void cleanup_next_depth(struct queue *ndheadp)
{
    struct queue *qp, *qpsave;
    struct put *putp;
    

    qp = GET_TOP_ELEMENT(*ndheadp);
    while (!IS_ENDQ(qp, *ndheadp)) {
        qpsave = qp->next;
        
        putp = DEPTH_TO_PUT(qp);
        
        if (!IS_EMPTYQ(putp->next_depth)) {
            cleanup_next_depth(&(putp->next_depth));
        }
        dprintf("deleting put=%p, (%c%d)\n", putp,
               (int)putp->p.x + 'A' , putp->p.y + 1);
        if (putp->bp != NULL) {
            cleanup_board(putp->bp);
        }
        delete(&(putp->candidate));
        delete(&(putp->depth));
        freeput(putp);
        qp = qpsave;
    }
    return;
}

void cleanup_depth(struct queue *dpheadp)
{
    struct queue *qp, *qpsave;
    struct depth *dp;

    qp = GET_LAST_ELEMENT(*dpheadp);
    while (1) {
        dp = Q_TO_DEPTH(qp);
        qpsave = qp->prev;
        if (dp->depth == 1) {
            return;
        }
        dprintf("deleting dp=%p depth=%d\n", dp, dp->depth);
        delete(&(dp->q));
        free(dp);
        qp = qpsave;
    }
}

        


#if 0
void cleanup_depth(struct depth *dp)
{
    struct queue *qp, *qsavep;
    struct queue *ndqp, *ndqpsave;
    struct put *putp, *ndputp;
    int i=0;

    /* get top element of candidate (struct put) queue */
    qp = GET_TOP_ELEMENT(dp->candidate);

    printf("cleanup_depth: depth=%d start.\n", dp->depth);
    while (!IS_ENDQ(qp, dp->candidate)) {
        qsavep = qp->next;
        putp = CANDIDATE_TO_PUT(qp);
        if (!IS_EMPTYQ(putp->next_depth)) {
            /* recursive call */;
            printf("next_depth is not empty. calling recursively.\n");
            cleanup_depth(Q_TO_DEPTH(dp->q.next));
        }
        /* cleanup */


        ndqp = GET_TOP_ELEMENT(putp->next_depth);
        while (!IS_ENDQ(ndqp, putp->next_depth)) {
            ndqpsave = ndqp->next;
            ndputp = DEPTH_TO_PUT(ndqp);
            printf("deleting put=%p, (%c%d)\n", ndqp,
                         (int)ndputp->p.x + 'A' , ndputp->p.y + 1);
#if 1
            if (ndputp->bp != NULL)
                cleanup_board(ndputp->bp);
            delete(&(ndputp->candidate));
            delete(&(ndputp->depth));
            freeput(ndputp);
#endif
            ndqp = ndqpsave;
        }

        qp = qsavep;
    }
    if (IS_EMPTYQ(dp->candidate)) {
        printf("deleting depth=%p (%d)\n", dp, dp->depth);
        delete(&(dp->q));
        free(dp);
    }
    printf("cleanup_depth: return depth=%d end.\n", dp->depth);
    return;
}
#endif


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

int choose_corner_border(struct session *sp,
                         struct queue *candidatep,
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
            ((putp->p.x == 0) && (putp->p.y == MAX_Y(sp))) ||
            ((putp->p.x == MAX_X(sp)) && (putp->p.y == 0)) ||
            ((putp->p.x == MAX_X(sp)) && (putp->p.y == MAX_Y(sp)))) {
                /* corner */
            delete(&(putp->candidate));
            append(cornerp, &(putp->candidate));

        } else  if ((putp->p.x == 0) || (putp->p.x == MAX_X(sp)) ||
                    (putp->p.y == 0) || (putp->p.y == MAX_Y(sp))) {
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
        push(&(sp->player[sp->turn].candidate), &(gputp->candidate));
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
        
        
        if (CELL(*bp, x, y) == p->color) {
            /* SAME COLOR */
            if (before_myself == YES) {
                ;
                
            } else {
                ;
                
            }
            
        } else if (CELL(*bp, x, y) == OPPOSITE_COLOR(p->color)) {
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
            if (CELL(*bp, x, y) == p->color) {
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
int think_level0(struct session *sp, struct put *p, int color)
{
    int x, y, retcode = NO;
    struct put *putp;
    struct board *bp = &(sp->bd);
    
    putp = allocput();
    for (y = 0; y < sp->bd.xsize; y++) {
        for (x = 0; x < sp->bd.ysize; x++) {
            initput(putp);
            putp->color = color;
            putp->p.y = y;
            putp->p.x = x;
            if (check_puttable(sp, bp, putp, color) == YES) {
                tdprintf("think_level0: found puttable place x=%d/y=%d,gettable=%d\n",
                         x, y, putp->gettable);
                *p = *putp;
                retcode = YES;
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
int think_level1(struct session *sp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount;
    struct put *putp, *gputp;
    struct queue *qp;
    struct board *bp = &(sp->bd);
    
    printf("think_level1: called (was_pass:%d)\n", sp->was_pass);
    
    pcount = simple_search_candidate(sp, color, &(sp->player[sp->turn].candidate));
    printf("think_level1: found %d cells.\n", pcount);
    if (pcount > 0) {
        retcode = YES;
    } else {
        /*
         * PASS
         */
        return NO;
    }
    *p = *choose_max_gettable(&(sp->player[sp->turn].candidate));
    INITQ(p->candidate);  /* ugly */

    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    
    cleanup_candidates(&(sp->player[sp->turn].candidate));
    dprintf("think_level1: return\n");
    
    return retcode;
}

/*
 * (2) level 2 --- enhancement of (1) that is simple depth 1 search.
 *   search all from x=0,y=0, depth = 1, and
 *   choose the cell by which the number of cells I can get
 *   will be maximized, and if there is corner stone among possible cells,
 *   choose them in preference to ones that enables more cells.
 */
int think_level2(struct session *sp, struct put *p, int color)
{
    int x, y, retcode = NO, pcount;
    struct put *putp, *gputp, *cgputp, *bgputp;
    struct queue *qp, corner, border;
    struct board *bp = &(sp->bd);
    
    pcount = simple_search_candidate(sp, color,
                                     &(sp->player[sp->turn].candidate));
    tdprintf("think_level2: found %d cells.\n", pcount);
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
    
    choose_corner_border(sp, &(sp->player[sp->turn].candidate),
                         &corner, &border);
#if 1
    /* max gettable cell on neither corner nor border is the top of cand. */
    gputp = CANDIDATE_TO_PUT(GET_TOP_ELEMENT((sp->player[sp->turn].candidate)));
#else
    gputp = choose_max_gettable(&candidate);
#endif
    
    dprintf("corner\n");
    cgputp = choose_max_gettable(&corner);

    dprintf("border\n");
    bgputp = choose_max_gettable(&border);

    dprintf("%p,%p,%p\n", cgputp, bgputp, gputp);
    
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
    
    cleanup_candidates(&(sp->player[sp->turn].candidate));
    cleanup_candidates(&corner);
    cleanup_candidates(&border);
    dprintf("think_level2 return\n");
    
    return retcode;
}

/*
 * (3) level 3 --- enhancement of (2) so that if the computer chooses
 *     a cell on border and it would be gotten by other player,
 *     avoid it. In case of no choice other than that,
 *     choose maximum gettable cell.
 */
int think_level3(struct session *sp, struct put *p, int color)
{
    int x, y, retcode, pcount;
    struct put *putp, *gputp, *cgputp, *bgputp;
    struct queue *qp, *qpsave, corner, border;
    struct board *bp = &(sp->bd);    

    pcount = simple_search_candidate(sp, color,
                                     &(sp->player[sp->turn].candidate));
    tdprintf("think_level3: found %d cells.\n", pcount);
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
    choose_corner_border(sp,
                         &(sp->player[sp->turn].candidate), &corner, &border);
    /* neigher corner nor border*/
    if (!IS_EMPTYQ(sp->player[sp->turn].candidate)) {
        gputp =
            CANDIDATE_TO_PUT(GET_TOP_ELEMENT(sp->player[sp->turn].candidate));
    }
    
    dprintf("corner\n");
    cgputp = choose_max_gettable(&corner);

    dprintf("border\n");
    bgputp = NULL;
    if (!IS_EMPTYQ(border)) {
        qp = GET_TOP_ELEMENT(border);
/*        bgputp = CANDIDATE_TO_PUT(qp);*/
        while (qp != &border) {
            putp = CANDIDATE_TO_PUT(qp);
            dprintf("border check level=%d, %08x, (%d,%d)\n",
                   sp->player[sp->turn].level,
                    putp->neighbor.i, putp->p.x, putp->p.y);
            if ((putp->p.x == 0) || (putp->p.x == MAX_X(sp))) {
                dprintf("border check both sides x=%d\n", putp->p.x);
                if (CHECK_NEIGHBOR(putp, UP) &&
                    CHECK_NEIGHBOR(putp, DOWN) &&
                    (CELL(*bp, putp->p.x, putp->p.y - 1) ==
                     CELL(*bp, putp->p.x, putp->p.y + 1))) {
                    dprintf("Both of up and down exist.\n");
                        
                    bgputp = putp;
                        
                } else if (CHECK_NEIGHBOR(putp, UP)) {
                    if (CELL(*bp, putp->p.x, putp->p.y - 1)
                            != OPPOSITE_COLOR(color)) {
                        dprintf("choose border stone%d/%d\n",
                               putp->p.x, putp->p.y);
                        bgputp = putp;
                    }
                } else if (CHECK_NEIGHBOR(putp, DOWN)) {
                    if (CELL(*bp, putp->p.x, putp->p.y + 1)
                        != OPPOSITE_COLOR(color)) {
                        dprintf("choose border stone%d/%d\n",
                               putp->p.x, putp->p.y);
                        bgputp = putp;
                    }
                } else {
                    dprintf("clean border cell %x/%x\n",putp->p.x, putp->p.y);
                    bgputp = putp;
                }
            }
            if ((putp->p.y == 0) || (putp->p.y == MAX_Y(sp))) {
                dprintf("hara\n");
                    
                if (CHECK_NEIGHBOR(putp, LEFT) &&
                    CHECK_NEIGHBOR(putp, RIGHT) &&
                    (CELL(*bp, putp->p.x, putp->p.y - 1) ==
                     CELL(*bp, putp->p.x, putp->p.y + 1))) {
                    dprintf("hoge");
                    bgputp = putp;
                    
                } else if (CHECK_NEIGHBOR(putp, LEFT)) {
                    if (CELL(*bp, putp->p.x - 1, putp->p.y)
                        != OPPOSITE_COLOR(color)) {
                        bgputp = putp;
                    }
                } else if (CHECK_NEIGHBOR(putp, RIGHT)) {
                    if (CELL(*bp, putp->p.x + 1, putp->p.y)
                        != OPPOSITE_COLOR(color)) {
                        bgputp = putp;
                    }
                } else {
                    dprintf("clean border cell %x/%x\n",putp->p.x, putp->p.y);
                    bgputp = putp;
                }
            }
            qp = qp->next;
        }
    }
    dprintf("%p,%p,%p\n", cgputp, bgputp, gputp);
    
    if (cgputp != NULL) {
            /* If a puttalbe coner cell exists,
             cgputp MUST NOT be NULL. */
        *p = *cgputp;
    
    } else if (bgputp != NULL) {
        *p = *bgputp;
        
    } else if (gputp == NULL) {
        if (!IS_EMPTYQ(border)) {
            dprintf("think_level3: It's rare!\n");
            *p = *choose_max_gettable(&border);
        } else {
            tdprintf("think_level3: internal inconsistency\n");
            return NO;
        }
        
    } else {
        *p = *gputp;
    }
    
    INITQ(p->candidate);  /* ugly */
    
    printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    
    cleanup_candidates(&(sp->player[sp->turn].candidate));
    cleanup_candidates(&corner);
    cleanup_candidates(&border);
    dprintf("think_level3 return\n");

    return retcode;
}

/*****************************************************************************/
/*
 * for level 4
 */
/*****************************************************************************/
int search_next_depth(struct session *sp, int myself, int this_turn)
{
    int pcount, accum = 0;
    struct board *bp;
    struct depth *dp, *predp;
    struct put *putp;
    struct queue *qp;

    dprintf("search_next_depth: myself=%d,this_turn=%d\n", myself, this_turn);
    predp = (struct depth *)GET_LAST_ELEMENT(sp->player[myself].depth);
    /*
     * debug information
     */
    if (0) {
        struct put *tputp;
        struct queue *q;
        
        q = GET_TOP_ELEMENT(predp->candidate);
        while (!IS_ENDQ(q, predp->candidate)) {
            tputp = CANDIDATE_TO_PUT(q);
            printf("search_next_depth: candidate is (%c%d)\n",
                   (int)tputp->p.x + 'A',
               tputp->p.y + 1);
            q = q->next;
        }
    }
    /*
     * allocate struct depth
     */
    if ((dp = alloc_depth()) != NULL) {
        init_depth(dp, predp->depth + 1);
        append(&(sp->player[sp->turn].depth), &(dp->q));
    } else {
        printf("search_next_depth: alloc_depth() failed.\n");
        return FAIL;
    }
    /*
     * process each candidate of the current depth
     * sp->player[myself].depth
     */
    qp = GET_TOP_ELEMENT(predp->candidate);
    while (!IS_ENDQ(qp, predp->candidate)) {
        putp     = CANDIDATE_TO_PUT(qp);
        dprintf("search_next_depth: checking next depth of candidate (%c%d) for %d\n",
               (int)putp->p.x + 'A', putp->p.y + 1, this_turn);
        dprintf("search_next_depth: The board with this put is...(bp=%p)\n",
               putp->bp);
#if 0
        output(putp->bp);
#endif
        
        pcount = simple_search_candidate_mp(sp,
                                            putp->bp,
                                            this_turn,
                                            &(dp->candidate),
                                            MULTIPLE_DEPTH,
                                            &(putp->next_depth));
        dprintf("search_next_depth: found %d cells. (depth=%d)\n",
               pcount, dp->depth);
        accum += pcount;
        qp = qp->next;
    }
    
    return accum;
}




void print_candidate_tree(struct queue *ndheadqp, struct depth *dp)
{
    struct queue *q1;
    struct put   *p1;
    
    q1 = GET_TOP_ELEMENT(*ndheadqp);
    while (!IS_ENDQ(q1, *ndheadqp)) {
        int i = dp->depth;
        
        p1 = DEPTH_TO_PUT(q1);
        while (i > 0) {
            printf("  ");
            i--;
        }
        
        printf("level=%d candidate (%c%d) n:%08x, c:%08x\n",
               dp->depth,
               (int)p1->p.x + 'A', p1->p.y + 1,
               p1->neighbor.i, p1->canget.i);
        if (!IS_EMPTYQ(p1->next_depth)) {
              print_candidate_tree(&(p1->next_depth), (struct depth *)(dp->q.next));
        }
        q1 = q1->next;
    }
    
}


int think_level4(struct session *sp, struct put *p, int color)
{
    int x, y, retcode, pcount, ret;
    struct put *putp; /* , *gputp, *cgputp, *bgputp;*/
    struct queue *qp, *qpsave, corner, border;
    struct board *bp = &(sp->bd);
    int depth, this_turn;
    struct depth *dp;

    printf("think_level4: depth=%d (color:%d/turn:%d)\n",
           sp->cfg.depth, color, sp->turn);
    

    /*
     * depth=1 search
     */
    this_turn = color;
    depth = 1;
#if 0
    if ((dp = alloc_depth()) != NULL) {
        init_depth(dp, 1);
        append(&(sp->player[sp->turn].depth), &(dp->q));
    } else {
        printf("think_level4: alloc_depth() failed.\n");
        return FAIL;
    }
#endif
    
    dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
    
    printf("think_level4: Checking depth=%d (this_turn=%d)\n",
           depth, this_turn);
    pcount = simple_search_candidate_mp(sp,
                                        &(sp->bd),
                                        this_turn,
                                        &(dp->candidate),
                                        MULTIPLE_DEPTH,
                                        &(dp->next_depth)); /* DUMMY */
#if 0
    qp = GET_TOP_ELEMENT(dp->candidate);
    while (!IS_ENDQ(qp, dp->candidate)) {
        putp = CANDIDATE_TO_PUT(qp);
        putp->bp = dup_board(sp, &(sp->bd));
        SET_CELL(*(putp->bp), putp->p.x, putp->p.y, putp->color);
        process_put_bd(sp, putp->bp, putp, putp->color);
        qp = qp->next;
    }
#endif
    tdprintf("think_level4: found %d cells for depth=%d\n", pcount, depth);
    if (pcount > 0) {
        retcode = YES;
    } else {
        /*
         * PASS
         */
        return NO;
    }
    /*
     * depth > 1 search
     */
    this_turn = OPPOSITE_COLOR(sp->turn);
    depth++;
    while (depth <= sp->cfg.depth) {
        printf("think_level4: Checking depth=%d (this_turn=%d)\n", depth, this_turn);
        ret = search_next_depth(sp, sp->turn, this_turn);
        if (ret == FAIL) {
            printf("think_level4: search_next_depth() failed.\n");
            return NO;
        }
        tdprintf("think_level4: search_next_depth() found %d cells for depth=%d.\n", ret, depth);
        
        this_turn = OPPOSITE_COLOR(this_turn);
        depth++;
    }

#if 0    
    if (1) {
        struct queue *q;
        struct depth *d;
        
        q = GET_TOP_ELEMENT((sp->player[sp->turn].depth));
        d = Q_TO_DEPTH(q);
        print_candidate_tree(&(d->next_depth), d);
    }
#endif
    /*
     * evaluate candidate tree
     */
    *p = *choose_max_gettable(&(dp->candidate));



    /*
     * cleanup
     */
#if 0
    cleanup_depth(Q_TO_DEPTH(sp->player[sp->turn].depth.next));
#else
    cleanup_next_depth(&((Q_TO_DEPTH(sp->player[sp->turn].depth.next))->next_depth));
    cleanup_depth(&(sp->player[sp->turn].depth));
#endif
    return retcode;
}


#if 0    
    if (0) {
        struct queue *q, *q1, *q2;
        struct depth *d;
        struct put   *p, *p1, *p2, *p3;
        
        q = GET_TOP_ELEMENT((sp->player[sp->turn].depth));
        while (!IS_ENDQ(q, (sp->player[sp->turn].depth))) {
            d = Q_TO_DEPTH(q);
            printf("depth=%d, d=%p, candidate:%d, d->next_depth:%d\n",
                   d->depth, d,
                   IS_EMPTYQ(d->candidate), IS_EMPTYQ(d->next_depth));
            if (d->depth == 1) {
                q1 = GET_TOP_ELEMENT(d->candidate);
                while (!IS_ENDQ(q1, d->candidate)) {
                    p1 = CANDIDATE_TO_PUT(q1);
                    printf("top level candidate (%c%d)\n",
                           (int)p1->p.x + 'A', p1->p.y + 1);
                    dprintf("next_depth:%p, next:%p, prev:%p, nextnext:%p, nextprev:%p\n",
                           p1->next_depth,
                           p1->next_depth.next, p1->next_depth.prev,
                           p1->next_depth.next->next,
                           p1->next_depth.next->prev);
                    
                    q2 = GET_TOP_ELEMENT(p1->next_depth);
                    while (!IS_ENDQ(q2, p1->next_depth)) {
                        p2 = DEPTH_TO_PUT(q2);
                        printf("  next level candidate (%c%d) %d\n",
                               (int)p2->p.x + 'A', p2->p.y + 1,
                               IS_EMPTYQ(p2->next_depth));

                        q2 = q2->next;
                    }

                    
                    q1 = q1->next;
                }
            }
            q = q->next;
        }
        

        
        
    }
#endif
    

int think(struct session *sp, struct put *p, int color)
{
    int retcode;
    struct board *bp = &(sp->bd);
    printf("think: level=%d\n", sp->player[sp->turn].level);

    switch (sp->player[sp->turn].level) {
    case 0:
        retcode = think_level0(sp, p, color);
        break;

    case 1:
        retcode = think_level1(sp, p, color);
        break;

    case 2:
        retcode = think_level2(sp, p, color);
        break;

    case 3:
        retcode = think_level3(sp, p, color);
        break;

    case 4:
        retcode = think_level4(sp, p, color);
        break;

    default:
        printf("level=%d is not implemented yet.\n",
               sp->player[sp->turn].level);
        exit(255);
    }
    return retcode;
}


