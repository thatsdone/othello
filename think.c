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

void freeboard(struct board *bp)
{
    free(bp->b);
    free(bp);
}

void cleanup_boards(struct depth *dp)
{
    struct queue *qp;
    struct put *putp;
    qp = GET_TOP_ELEMENT(dp->candidate);
    while (IS_ENDQ(qp, dp->candidate)) {
        putp = CANDIDATE_TO_PUT(qp);
        freeboard(putp->bp);
        putp->bp = NULL;
        qp = qp->next;
    }
}


#define SINGLE_DEPTH 0
#define MULTIPLE_DEPTH 1

int simple_search_candidate(struct session *sp, int color, struct queue *head)
{
    struct board *bp = &(sp->bd);
    return simple_search_candidate_mp(sp, bp, color, head, SINGLE_DEPTH, NULL);
}

#define NEXTDEPTH_TO_PUT(q) (struct put *)((char *)q - \
        offsetof(struct put, next_depth))

int simple_search_candidate_mp(struct session *sp,
                               struct board *bp,
                               int color,
                               struct queue *candidatep,
                               int mode,
                               struct queue *next_depthp)
{
    int x, y, pcount;
    struct put *putp, *wputp;
    
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
                    SET_CELL(*(putp->bp), putp->p.x, putp->p.y, putp->color);
                    process_put(sp, putp->bp, putp, putp->color);
                    dprintf("simple_search_candidate_mp: %p\n", putp->bp);
#if 0
                    putp->up = NEXTDEPTH_TO_PUT(next_depthp);
#endif                    
                }
                putp = allocput();
            }
        }
    }
    freeput(putp);
        
    return pcount;
}



void cleanup_candidates(struct queue *canp)
{
    struct queue *qp, *qsavep;
    struct put *putp;
    
    qp = GET_TOP_ELEMENT(*canp);
    while (qp != canp) {
        qsavep = qp->next;
        dprintf("deleting qp=%p\n", qp);
        putp = CANDIDATE_TO_PUT(qp);
        delete(&putp->candidate);
        delete(&putp->depth);
        if (putp->bp != NULL) {
            freeboard(putp->bp);
        }
        putp->bp = NULL;
        freeput(putp);
        qp = qsavep;
    }
    return;
}


/*
 * cleanup_next_depth()
 *   cleanups struct put tree from upper nodes recursively
 *   ndheadp is head queue of next_depth in the top struct depth.
 */
void cleanup_next_depth(struct put *putp)
{
    struct queue *qp, *qpsave, *tqp, *tqpsave, *tqheadp;
    struct put *tputp;
    dprintf("cleanup_next_depth\n");
    if (IS_EMPTYQ(putp->next_depth)) {
        /*
         * no more next_depth
         */
        dprintf("put=%p, (%c%d) has no more next_depth.\n", putp,
               (int)putp->p.x + 'A' , putp->p.y + 1);
        
        return;
        
    } else {
        /*
         * more next_depth puts exists
         */
        qp = GET_TOP_ELEMENT(putp->next_depth);        
        while (!IS_ENDQ(qp, putp->next_depth)) {
            qpsave = qp->next;
            tputp = DEPTH_TO_PUT(qp);
            cleanup_next_depth(tputp);
            dprintf("cleaning up put=%p, (%c%d).\n", tputp,
                    (int)tputp->p.x + 'A' , tputp->p.y + 1);
                delete(&(tputp->candidate));
                delete(&(tputp->depth));
                if (tputp->bp != NULL){
                    freeboard(tputp->bp);
                }
                tputp->bp = NULL;
                freeput(tputp);
            qp = qpsave;
        }
    }
    
    return;
}


void cleanup_next_depth_all(struct queue *ndheadp)
{
    struct queue *qp, *qpsave, *tqp, *tqpsave, *tqheadp;
    struct put *putp, *tputp;
    dprintf("cleanup_next_depth_all\n");
    
    qp = GET_TOP_ELEMENT(*ndheadp);
    while (!IS_ENDQ(qp, *ndheadp)) {
        qpsave = qp->next;
        
        putp = DEPTH_TO_PUT(qp);

        cleanup_next_depth(putp);

        delete(&(putp->candidate));
        delete(&(putp->depth));
        if (putp->bp != NULL)
            freeboard(putp->bp);
        freeput(putp);

        qp = qpsave;
    }
    return;
}
        

void cleanup_depth(struct queue *dpheadp)
{
    struct queue *qp, *qpsave;
    struct depth *dp;
    int depth;
    
    
    qp = GET_LAST_ELEMENT(*dpheadp);
    while (1) {
        dp = Q_TO_DEPTH(qp);
        depth = dp->depth;
        qpsave = qp->prev;
        dprintf("deleting dp=%p depth=%d\n", dp, dp->depth);
        delete(&(dp->q));
        free(dp);
        if (depth == 1) {
            return;
        }
        qp = qpsave;
    }
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

int choose_corner_border(struct session *sp,
                         struct queue *candidatep,
                         struct queue *cornerp,
                         struct queue *borderp)
{
    int retcode = YES;
    struct queue *qp, *qpsave;
    struct put *putp, *gputp;
    
    qp = GET_TOP_ELEMENT(*candidatep);
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
#if 0
        if (((putp->p.x == 0) && (putp->p.y == 0)) ||
            ((putp->p.x == 0) && (putp->p.y == MAX_Y(sp))) ||
            ((putp->p.x == MAX_X(sp)) && (putp->p.y == 0)) ||
            ((putp->p.x == MAX_X(sp)) && (putp->p.y == MAX_Y(sp)))) {
#endif
        if (IS_CORNER(sp, putp)) {
                
                /* corner */
            delete(&(putp->candidate));
            append(cornerp, &(putp->candidate));

#if 0
        } else  if ((putp->p.x == 0) || (putp->p.x == MAX_X(sp)) ||
                    (putp->p.y == 0) || (putp->p.y == MAX_Y(sp))) {
#endif
        } else  if (IS_BORDER(sp, putp)) {
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
int search_next_depth(struct session *sp, int depth, int myself, int this_turn)
{
    int pcount, accum = 0;
    struct board *bp;
    struct depth *dp, *predp;
    struct put *putp;
    struct queue *qp;

    dprintf("search_next_depth: myself=%d,this_turn=%d\n", myself, this_turn);
    /*
     * process each candidate of the current depth
     * sp->player[myself].depth
     */
    dprintf("search_next_depth: enter: num_put is %d\n", num_put);
    if (depth == 1) {
        /*
         * depth = 1
         * In this case, 
         */
        if ((dp = allocdepth()) != NULL) {
            initdepth(dp, depth);
            append(&(sp->player[myself].depth), &(dp->q));
        } else {
            printf("search_next_depth: allocdepth() failed.\n");
            return FAIL;
        }
        dprintf("search_next_depth: checking depth=%d (this_turn=%d)\n",
               depth, this_turn);
        pcount = simple_search_candidate_mp(sp,
                                            &(sp->bd),
                                            this_turn,
                                            &(dp->candidate),
                                            MULTIPLE_DEPTH,
                                            &(dp->next_depth)); /* DUMMY */
        dprintf("search_next_depth: found %d cells (depth=%d)\n",
                 pcount, depth);
        dp->num_cand = pcount;
#if 0
        if (pcount > 0) {
#endif
            accum += pcount;
#if 0
        } else {
            /*
             * PASS
             */
            return accum;
        }
#endif        
    } else {
        /*
         * depth > 1 
         */
        predp = Q_TO_DEPTH(GET_LAST_ELEMENT(sp->player[myself].depth));
        if ((dp = allocdepth()) != NULL) {
            initdepth(dp, depth);
            append(&(sp->player[myself].depth), &(dp->q));
        } else {
            printf("search_next_depth: allocdepth() failed.\n");
            return FAIL;
        }
        qp = GET_TOP_ELEMENT(predp->candidate);
        while (!IS_ENDQ(qp, predp->candidate)) {
            putp = CANDIDATE_TO_PUT(qp);
            dprintf("search_next_depth: checking next depth of candidate (%c%d) for %d\n",
                    (int)putp->p.x + 'A', putp->p.y + 1, this_turn);
#if 0
            dprintf("search_next_depth: The board with this put is.(bp=%p)\n",
                    putp->bp);
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
            dp->num_cand = pcount;            
            qp = qp->next;
        }
        /*
         * surpress memory comsumption. :(
         */
#if 0
        cleanup_boards(predp);
#endif        
    }
    dprintf("search_next_depth: return: num_put is %d\n", num_put);    
    return accum;
}




void print_candidate_tree(struct queue *ndheadqp, struct depth *dp)
{
    struct queue *q;
    struct put   *p;
    
    q = GET_TOP_ELEMENT(*ndheadqp);
    while (!IS_ENDQ(q, *ndheadqp)) {
        int i = dp->depth;
        
        p = DEPTH_TO_PUT(q);
        while (i > 0) {
            printf("  ");
            i--;
        }
        
        if (!IS_EMPTYQ(p->next_depth)) {
              print_candidate_tree(&(p->next_depth),
                                   Q_TO_DEPTH(dp->q.next));
        }
        q = q->next;
    }
    
}

#define STRATEGY_RANDOM           0x0001
#define STRATEGY_MINI_MAX         0x0002
#define STRATEGY_MINI_MIN         0x0004
#define STRATEGY_CENTER_ORIENTED  0x0008
#define STRATEGY_LIMB_ORIENTED    0x0010

#define STRATEGY_CORNER_BORDER    0x0020

#define STRATEGY_GA               100
#define STRATEGY_NEURALNET        101


int is_stable_cell(struct put *putp)
{
    int retcode = NO;
    struct queue *qp1;
    struct put *putp1;
    
    if (!IS_EMPTYQ(putp->next_depth)) {
        qp1 = GET_TOP_ELEMENT(putp->next_depth);
        while (!IS_ENDQ(qp1, putp->next_depth)) {
            putp1 = DEPTH_TO_PUT(qp1);
            dprintf("put1: %d/%d\n",
                    CELL(*putp1->bp, putp->p.x, putp->p.y),
                    putp1->color);
            
            if (CELL(*putp1->bp, putp->p.x, putp->p.y)
                != putp->color) {
                printf("This cell must be caught by enemy.\n");
                return retcode;
            }
            qp1 = qp1->next;
        }
        retcode = YES;
        
    } else {
        retcode = YES;
    }
    
    
    return retcode;
}

#define CHECK_STRATEGY(input, str) (input & str)


struct put *simple_strategy(struct session *sp, unsigned int strategy)
{
    struct put *putp, *putp1;
    struct queue *qp, *qp1, *qpsave, qcorner, qborder, qcenter;
    struct depth *dp;
    int corner = NO, border = NO;

    INITQ(qcorner);
    INITQ(qborder);
    INITQ(qcenter);

    if (CHECK_STRATEGY(strategy, STRATEGY_CORNER_BORDER)) {
      
        dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
        qp = GET_TOP_ELEMENT(dp->candidate);
        while (!IS_ENDQ(qp, dp->candidate)) {
            qpsave = qp->next;
            putp = CANDIDATE_TO_PUT(qp);
            if (IS_CORNER(sp, putp)) {
                tdprintf("found puttable cell on corners.\n");
                delete(&(putp->candidate));
                append(&qcorner, &(putp->candidate));
                goto exit;
            
                corner = YES;
            }
            if (IS_BORDER(sp, putp)) {
                tdprintf("found puttable cell (%c%d) on borders\n",
                       (int)putp->p.x + 'A', putp->p.y + 1);
                if(is_stable_cell(putp) == YES) {
                    
                    delete(&(putp->candidate));
                    append(&qborder, &(putp->candidate));
                    goto exit;
                    
                    border = YES;
                }
            }
            qp = qpsave;
        }
    }
 
 
    if (CHECK_STRATEGY(strategy, STRATEGY_RANDOM)) {
     
        if ((corner == NO) && (border == NO)) {
            int i, end, found = NO;
            tdprintf("No corner or border cell.\n");
                /*
                 * random
                 *   num_cand cannot not be 0, otherwise BUG
                 */
            end = orand(dp->num_cand);
            end = end % dp->num_cand + 1;
            dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
            qp = GET_TOP_ELEMENT(dp->candidate);
            i = 1;
            while (!IS_ENDQ(qp, dp->candidate)) {
                putp = CANDIDATE_TO_PUT(qp);
                    /* IS_BORDER picks up also corner cells */
                if ((i >= end) && !IS_BORDER(sp, putp)) {
                    found = YES;
                    break;
                }
                qp = qp->next;
                i++;
            }
            if (found == NO) {
                dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
                qp = GET_TOP_ELEMENT(dp->candidate);
                i = 1;
                while (!IS_ENDQ(qp, dp->candidate)) {
                    putp = CANDIDATE_TO_PUT(qp);
                    if (i >= end) {
                        found = YES;
                        break;
                    }
                    qp = qp->next;
                    i++;
                }
            }
            tdprintf("random candidate (%c%d), gettable=%d, random=%d\n",
                   (int)putp->p.x + 'A', putp->p.y + 1, putp->gettable, end);
        }
    }
    /* center oriented routine is not completed */    
    if (CHECK_STRATEGY(strategy, STRATEGY_CENTER_ORIENTED)) {
        
        if ((corner == NO) && (border == NO)) {
            int i, j, end;
            /*
             * center oriented randomlly
             */
            dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
            qp = GET_TOP_ELEMENT(dp->candidate);
            i = 0;
            while (!IS_ENDQ(qp, dp->candidate)) {
                qpsave = qp->next;
                dprintf("IS_CENTER is %d\n", IS_CENTER(sp, putp));
                if (IS_CENTER(sp, putp)) {
                    delete(&putp->candidate);
                    putp = CANDIDATE_TO_PUT(qp);                
                    append(&qcenter, &(putp->candidate));
                    i++;
                }
                qp = qpsave;
            }
            if (i == 0) {
                end = orand(i);
                end = end % i + 1;
                qp = GET_TOP_ELEMENT(qcenter);
                j = 0;
                while (!IS_ENDQ(qp, qcenter)) {
                    if (j >= end) {
                    break;
                    }
                    j++;
                    qp = qp->next;
                }
                putp = CANDIDATE_TO_PUT(qp);
                tdprintf("random candidate (%c%d), gettable=%d, random=%d\n",
                         (int)putp->p.x + 'A', putp->p.y + 1,
                         putp->gettable, end);
/*                i = dp->num_cand; */
            }
        }
    }

  exit:
    
    return putp;
}


int think_level4(struct session *sp, struct put *p, int color)
{
    int retcode, pcount, ret, totalcand = 0, restcells;
    struct put *putp;
    struct queue *qp;
    int depth, this_turn;
    struct depth *dp;

    tdprintf("think_level4: depth=%d (color:%d/turn:%d)\n",
           sp->cfg.depth, color, sp->turn);
    dprintf("num_put is %d\n", num_put);
    
    retcode = YES;
    restcells = sp->bd.xsize * sp->bd.ysize -
        get_occupied_cell_num(sp);
    dprintf("restcells = %d\n", restcells);
    
    /*
     * search multiple depths
     */
    this_turn = color;
    depth = 1;
    while ((depth <= sp->cfg.depth) && (depth <= restcells)) {
        tdprintf("think_level4: Checking depth=%d (this_turn=%d)\n",
               depth, this_turn);
        ret = search_next_depth(sp, depth, sp->turn, this_turn);
        if (ret == FAIL) {
            printf("think_level4: search_next_depth() failed.\n");
            exit(255);
            
        } else if ((depth == 1) && (ret == 0)) {
            /*
             * PASS
             */
            return NO;
        }
        
        tdprintf("think_level4: found %d cells for depth=%d.\n",
                 ret, depth);
        totalcand += ret;
        this_turn = OPPOSITE_COLOR(this_turn);
        depth++;
    }
    tdprintf("total cell number is %d.\n", totalcand);
    sp->player[sp->turn].num_candidate = totalcand;
    
    dprintf("num_put is %d\n", num_put);
    if (0) {
        struct queue *q;
        struct depth *d;
        d = Q_TO_DEPTH(GET_TOP_ELEMENT((sp->player[sp->turn].depth)));
        print_candidate_tree(&(d->next_depth), d);
    }

    /*
     * evaluate candidate tree
     */
    *p = *simple_strategy(sp, STRATEGY_CORNER_BORDER | STRATEGY_RANDOM);
    /*
     * show my put
     */
    tdprintf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    /*
     * cleanup
     */
    dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
    cleanup_next_depth_all(&(dp->next_depth));
    cleanup_depth(&(sp->player[sp->turn].depth));
    dprintf("num_put is %d\n", num_put);
    
    return retcode;
}
    


int think(struct session *sp, struct put *p, int color)
{
    int retcode;
    struct board *bp = &(sp->bd);
    tdprintf("think: level=%d\n", sp->player[sp->turn].level);

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


