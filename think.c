/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "othello.h"



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
    struct put *putp, *wputp;
    
    pcount = 0;
    putp = allocput();
    for (y = 0; y < sp->bd.ysize; y++) {
        for (x = 0; x < sp->bd.xsize; x++) {
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
                    if (next_depthp) {
                        append(next_depthp, &(putp->depth));
                        putp->up = NEXTDEPTH_TO_PUT(next_depthp);
                    }
                    putp->bp = dup_board(sp, bp); /* BUG ! for level 5 */
                    SET_CELL(*(putp->bp), putp->p.x, putp->p.y, putp->color);
                    process_put(sp, putp->bp, putp, putp->color);
                    dprintf("simple_search_candidate_mp: %p\n", putp->bp);

                    putp->up = NEXTDEPTH_TO_PUT(next_depthp);

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
    
    qp = GET_LAST_ELEMENT((*dpheadp));
    while (!IS_EMPTYQ(*dpheadp)) {
        dp = Q_TO_DEPTH(qp);
#if 0
        depth = dp->depth;
#endif
        qpsave = qp->prev;
#if 0
        dprintf("deleting dp=%p depth=%d\n", dp, dp->depth);
#endif
        delete(&(dp->q));
        free(dp);
#if 0
        if (depth == 1) {
            return;
        }
#endif
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
        if (IS_CORNER(sp, putp)) {
            /* corner */
            delete(&(putp->candidate));
            append(cornerp, &(putp->candidate));

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
    
    tdprintf("think_level1: called (was_pass:%d)\n", sp->was_pass);
    
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
                                            &(sp->player[sp->turn].next_depth));

        dprintf("search_next_depth: found %d cells (depth=%d)\n",
                 pcount, depth);
        dp->num_cand = pcount;
        accum += pcount;
        
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
            printf("search_next_depth: found %d cells. (depth=%d)\n",
                    pcount, depth);
            accum += pcount;
            dp->num_cand = pcount;            
            qp = qp->next;
        }
        /*
         * surpress memory comsumption. :(
         */
        if (0) {
            cleanup_boards(predp);
        }
    }
    dprintf("search_next_depth: return: num_put is %d\n", num_put);    
    return accum;
}



void print_candidate_tree(struct queue *ndheadqp, int depth)
{
    int black, white;
    struct queue *q;
    struct put   *p;
    dprintf("print_candidate_tree called %d\n", depth);
    
    q = GET_TOP_ELEMENT(*ndheadqp);
    while (!IS_ENDQ(q, *ndheadqp)) {
        int i = depth;
        
        p = DEPTH_TO_PUT(q);
        while (i > 0) {
            printf("  ");
            i--;
        }
        if (p->bp != NULL) {
            calculate_score(p->bp, &black, &white);
        } else {
            black = 0;
            white = 0;
            printf("p->bp is NULL!\n");
        }
        
                
        
        if (p->p.x != -1) {
            printf("depth(%d)  %s(%c%d) (B/W)=(%d/%d) p:%p/p->cand:%p\n",
                   depth,
                   (p->color == BLACK ? "B" : "W"),
                   (int)p->p.x + 'A', p->p.y + 1,
                   black, white, p, &(p->candidate));
        } else {
            printf("depth(%d)  %s(PASS) (B/W)=(%d/%d)\n",
                   depth,
                   (p->color == BLACK ? "B" : "W"),
                   black, white);
            
        }
        if (!IS_EMPTYQ(p->next_depth)) {
              print_candidate_tree(&(p->next_depth),
                                   depth + 1);
        }
        if (q->next == q) {
            printf("abnormal queue \n");
            exit(255);
        }
        
        q = q->next;
    }
    
}



int count_border_cell(struct session *sp, struct board *bp, int color)
{
    int count = 0, i;
    /*
     * this routine assumes a square board
     */

    if (CELL(*bp, 0, 0)                 == color) count++;
    if (CELL(*bp, 0, MAX_Y(sp))         == color) count++;
    if (CELL(*bp, MAX_X(sp), 0)         == color) count++;        
    if (CELL(*bp, MAX_X(sp), MAX_Y(sp)) == color) count++;
    for (i = 1; i < sp->cfg.boardsize - 1; i++) {
        if (CELL(*bp, i, 0)         == color) count++;
        if (CELL(*bp, i, MAX_Y(sp)) == color) count++;
        if (CELL(*bp, 0, i)         == color) count++;
        if (CELL(*bp, MAX_X(sp), i) == color) count++;        
    }
    
    return count;
}


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
#if 0
    if (sp->player[sp->turn].level == 5) {
        /*
         * test for level 5
         *   experiment for searching border gettalbe cell in depth=3 future
         *   but no good. :(
         */
        printf("level5 \n");
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
            } else 
            if (IS_BORDER(sp, putp)) {
                tdprintf("found puttable cell (%c%d) on borders\n",
                       (int)putp->p.x + 'A', putp->p.y + 1);
                if(is_stable_cell(putp) == YES) {
                    
                    delete(&(putp->candidate));
                    append(&qborder, &(putp->candidate));
                                    goto exit;                    
                    border = YES;
                }
            } else {
                struct queue *wqp1, *wqp2;
                struct put *wputp1, *wputp2;
                

                    wqp1 = GET_TOP_ELEMENT(putp->next_depth);
                    while (!IS_ENDQ(wqp1, putp->next_depth)) {
                        wputp1 = DEPTH_TO_PUT(wqp1);
                        if (!IS_EMPTYQ(wputp1->next_depth)) {
                            wqp2 = GET_TOP_ELEMENT(wputp1->next_depth);
                            while (!IS_ENDQ(wqp2, wputp1->next_depth)) {
                                wputp2 = DEPTH_TO_PUT(wqp2);
                                if (IS_BORDER(sp, wputp2) &&
                                    (CELL(*wputp2->bp, wputp2->p.x,
                                         wputp2->p.y) == sp->turn)) {
tdprintf("found puttable cell (%c%d) on borders on 2nd next depth of (%c%d)\n",
                         (int)wputp2->p.x + 'A', wputp2->p.y + 1,
                         (int)putp->p.x + 'A', putp->p.y + 1
                         );
                    /*
                exit(0);
                    */
    /*
                                    goto exit;
    */
                                }
                                wqp2 = wqp2->next;
                            }
                        }
                        wqp1 = wqp1->next;
                    }

            }
          
            qp = qpsave;
        }

    } else
#endif    
    
    if (CHECK_STRATEGY(strategy, STRATEGY_CORNER_BORDER)) {
        printf("count_border_cell = %d\n", count_border_cell(sp, &(sp->bd), 
               sp->turn));
        
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
            if (dp->num_cand == 0) {
                printf("Internal inconsistency: simple_strategy, num_cand=0\n");
                exit(255);
            }
            
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

int search_depth(struct session *sp, int max_depth)
{
    int this_turn, depth, ret, color;
    int restcells, totalcand;

    printf("search_depth called\n");
    
    restcells = sp->bd.xsize * sp->bd.ysize -
        get_occupied_cell_num(sp);
    dprintf("search_depth: restcells = %d\n", restcells);
    
    totalcand = 0;
    this_turn = sp->turn;
    depth = 1;
/*    while ((depth <= sp->cfg.depth) && (depth <= restcells)) { */
    while ((depth <= max_depth) && (depth <= restcells)) {    
        tdprintf("search_depth: Checking depth=%d (this_turn=%d)\n",
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
        
        tdprintf("search_depth: found %d cells for depth=%d.\n",
                 ret, depth);
        totalcand += ret;
        this_turn = OPPOSITE_COLOR(this_turn);
        depth++;
    }
    tdprintf("search_depth: total cell number is %d.\n", totalcand);
    sp->player[sp->turn].num_candidate = totalcand;

    return YES;
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

    ret = search_depth(sp, sp->cfg.depth);
    if (ret == NO) {
        retcode = NO;
        return retcode;
    }
    
    
#if 0    
    
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
#endif
    dprintf("num_put is %d\n", num_put);
    if (1) {
        struct queue *q;
        struct depth *d;
        struct put *wput;
        d = Q_TO_DEPTH(GET_TOP_ELEMENT((sp->player[sp->turn].depth)));
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
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

    cleanup_next_depth_all(&(sp->player[sp->turn].next_depth));
    cleanup_depth(&(sp->player[sp->turn].depth));
    dprintf("num_put is %d\n", num_put);
    
    return retcode;
}

/* ************************************************************************ */


void append_passput(struct session *sp, struct depth *dp,
                    struct queue *next_depthp, int color)
{
    struct put *putp;
    
    putp = allocput();
    putp->color = color;
    putp->p.x = -1;
    putp->p.y = -1;
#if 0
    dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
#endif
    append(&(dp->candidate), &(putp->candidate));
    append(next_depthp, &(putp->depth));
}


void cleanup_one_candidate_tree(struct put *putp)
{
    struct put *wputp;
    struct queue *wqp, *wqpsave;


    dprintf("cleanup_one_candidate_tree %p\n", putp);
    
    if (IS_EMPTYQ(putp->next_depth)) {
        return;
        
    } else {
        wqp = GET_TOP_ELEMENT(putp->next_depth);
        while (!IS_ENDQ(wqp, putp->next_depth)) {
            wqpsave = wqp->next;
            wputp = DEPTH_TO_PUT(wqp);
            cleanup_one_candidate_tree(wputp);
            dprintf("freeing1 put %p\n", wputp);
            delete(&(wputp->candidate));
            delete(&(wputp->depth));
            if (wputp->bp != NULL) {
                freeboard(wputp->bp);
                wputp->bp = NULL;
            }
            freeput(wputp);
            wqp = wqpsave;
        }
    }
#if 0    
    printf("freeing2 put %p\n", putp);    
    delete(&(putp->candidate));
    delete(&(putp->depth));
    if (putp->bp != NULL) {
        freeboard(putp->bp);
    }
    freeput(putp);
#endif
    return;
}

void cleanup_top_depth(struct session *sp, struct depth *dp, struct put *uputp)
{
    struct queue *qp, *qpsave;
    struct put *putp;

    dprintf("clanup_top_depth called. uputp=%p\n", uputp);
    
    delete(&(dp->q));
    qp = GET_TOP_ELEMENT(dp->candidate);
    dprintf("start from put %p\n", CANDIDATE_TO_PUT(qp));
    while (!IS_ENDQ(qp, dp->candidate)) {
        qpsave = qp->next;
        putp = CANDIDATE_TO_PUT(qp);
        if (putp != uputp) {
            cleanup_one_candidate_tree(putp);
#if 0
            printf("cleanup_top_depth: freeing put %p (%c%d)\n",
                   putp, putp->p.x + 'A', putp->p.y + 1);
#endif
            delete(&(putp->candidate));
            delete(&(putp->depth));
            if (putp->bp != NULL) {
                freeboard(putp->bp);
            }
            freeput(putp);

        } else {
            ;
        }
        qp = qpsave;
    }
#if 0
    {
        struct put *tputp;
    printf("before requeue_all: uputp->next_depth.next is %p, prev is %p\n",
           uputp->next_depth.next, uputp->next_depth.prev);
    tputp = DEPTH_TO_PUT(uputp->next_depth.next);
    printf("before: %p (%c%d) %d\n",
           tputp, tputp->p.x + 'A', tputp->p.y + 1, tputp->color);
    tputp = DEPTH_TO_PUT(uputp->next_depth.prev);
    printf("before: %p (%c%d) %d\n",
           tputp, tputp->p.x + 'A', tputp->p.y + 1, tputp->color);
    
    printf("freeing put %p (%c%d) %d\n", putp, putp->p.x + 'A', putp->p.y + 1, tputp->color);

    
    printf("before requeue_all. from is %p, to is %p\n",
           &(uputp->next_depth),
           &(sp->player[sp->turn].next_depth));
    }
    
#endif    

    delete(&(uputp->candidate));
    delete(&(uputp->depth));

    requeue_all(&(uputp->next_depth), &(sp->player[sp->turn].next_depth));

    
#if 0
    {
        struct put *tputp;
        
    tputp = DEPTH_TO_PUT(sp->player[sp->turn].next_depth.next);
    printf("after: %p (%c%d) %d\n",
           tputp, tputp->p.x + 'A', tputp->p.y + 1, tputp->color);
    tputp = DEPTH_TO_PUT(sp->player[sp->turn].next_depth.prev);
    printf("after: %p (%c%d) %d\n",
           tputp, tputp->p.x + 'A', tputp->p.y + 1, tputp->color);
    }
    
    printf("called requeue_all. from is %p, to is %p\n",
           &(uputp->next_depth),
           &(sp->player[sp->turn].next_depth));
#endif
    if (uputp->bp != NULL) {
        freeboard(uputp->bp);
        uputp->bp = NULL;
    }
    freeput(uputp);
    
    freedepth(dp);
    dprintf("clanup_top_depth return.\n");    
    return;
}



#define ONE_CANDIDATE    0x000001
#define EVERY_CANDIDATE  0x000002
#define START_ZERO       0x000010
#define START_FROM       0x000020
#define EXCEPT_GIVEN     0x000040

struct put *have_candidates(struct session *sp,
                            struct board *bp,
                            struct depth *dp,
                            struct queue *next_depthp,
                            struct put *startputp,
                            int this_turn,
                            int mode)
{
    int x, y, found = NO, accum = 0;
    struct put *putp, *first_putp = NULL;
    struct put *rputp;
    int startx, starty;
    int color;
    
    dprintf("have_candidates: start from (%c%d) mode %08x\n",
                         (int)startputp->p.x + 'A' , startputp->p.y + 1, mode);
    color = this_turn;
    /*
     * If START_FROM flag is on, adjust the starting cordinate of this search.
     */
    if (mode & START_FROM) {
        startx = startputp->p.x;
        starty = startputp->p.y;
    
        if (startputp->p.x >= MAX_X(sp)) {
            startx = 0;
            starty = startputp->p.y + 1;
            if (startputp->p.y >= MAX_Y(sp)) {
                    /* no more candidate */
                return NULL;
            }
        }
        
    } else {
        dprintf("have_candidates: search every cell\n");
        startx = 0;
        starty = 0;
    }
    dprintf("have_candidates: start put is  (%c%d)\n",
            startputp->p.x + 'A' , startputp->p.y + 1);
     /*
      * search (a) candidate(s) according to the mode flag.
      */
    putp = allocput();
    for (y = starty; y < sp->bd.ysize; y++) {
        for (x = startx ; x < sp->bd.xsize; x++) {
            initput(putp);
            putp->color = color;
            putp->p.x = x;
            putp->p.y = y;
            if ((mode & EXCEPT_GIVEN) &&
                IS_SAME_CELL(putp, startputp)) {
                printf("skipping the given cell (%c%d).\n",
                       (int)x + 'A' , y + 1);
#if 0
            } else {
#else
                continue;
            }
#endif
            
            if (check_puttable(sp, bp, putp, color) == YES) {
                found = YES;
                
                accum++;
                if (mode & EVERY_CANDIDATE)
                printf("have_candidates: found cell (%c%d)\n",
                         (int)x + 'A' , y + 1);
#if 0
                if ((mode & EXCEPT_GIVEN) && (mode & EVERY_CANDIDATE)) {
#else
                if ((mode & EVERY_CANDIDATE)) {
#endif
                    dprintf("next_depthp is %p\n", next_depthp);
                    if (next_depthp) {
                        append(next_depthp, &(putp->depth));
                    }
                    dprintf(
                       "have_candidates: appending put=%p(cand:%p) to dp=%p\n",
                       putp, &(putp->candidate), dp);
                    append(&(dp->candidate), &(putp->candidate));

                    putp->up = startputp;
                }

                putp->bp = dup_board(sp, bp);
#if 0
                if (mode & EVERY_CANDIDATE)
                    printf("putp is %p, bp is %p\n", putp, putp->bp);
#endif                
                SET_CELL(*(putp->bp), putp->p.x, putp->p.y, putp->color);
                process_put(sp, putp->bp, putp, putp->color);
                
                if (mode & EVERY_CANDIDATE) {
                    if (first_putp == NULL) {
                        first_putp = putp;
                    }
                    putp = allocput();
                } else {
                    return putp;
                }
#if 0
            }
#endif
            }
        }
    }
    if (found == NO) {
        putp->p.x = -1;
        putp->p.y = -1;
        
    }
    printf("EVERY_CANDIDATES: found %d cells\n", accum);

    if (mode & EVERY_CANDIDATE)
        freeput(putp);
    
    return first_putp;
}

int search_depth_vertical(struct session *sp,
                          int max_depth,
                          int start_depth,
                          int this_turn,
                          struct queue *next_depthp,
                          struct put *startputp)
{
    int ret;
    int retcode, count;
    struct put *wputp;
    struct queue *qp;
    struct depth *dp;
    
    
    dprintf("search_depth_vertical: called. depth/max=%d/%d, color=%d\n",
             start_depth, max_depth, this_turn);
    
    if (start_depth <= max_depth) {
        /*
         * search struct depth for this start_depth, and alocate one
         * if necessary.
         */
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        count = 0;
        dp = NULL;
        while (!IS_ENDQ(qp, sp->player[sp->turn].depth)) {
            count++;
            if (count == start_depth) {
                dp = Q_TO_DEPTH(qp);
                dprintf("found depth=%d dp=%p\n", start_depth, dp);
            }
            qp = qp->next;
        }
        dprintf("dp=%p, count=%d\n", dp, count);
        if (dp == NULL) {
            dprintf("allocating struct depth for depth=%d\n", start_depth);
            if ((dp = allocdepth()) != NULL) {
                initdepth(dp, start_depth);
                append(&(sp->player[sp->turn].depth), &(dp->q));
            } else {
                printf("search_next_depth: allocdepth() failed.\n");
                return FAIL;
            }
        }
        /*
         * search one candidate cell anyway.
         */
        wputp = have_candidates(sp,
                                startputp->bp,
                                dp,
                                next_depthp,
                                startputp,
                                this_turn,
                                ONE_CANDIDATE | START_ZERO);
        
        if (wputp != NULL) {
            /*
             *
             */
            dprintf("search_depth_vertical: checking recursively by(%c%d).\n",
               (int)wputp->p.x + 'A' , wputp->p.y + 1);
            
            dprintf("appending cand %p to dp %p\n",
                    &(wputp->candidate), dp);
            append(&(dp->candidate), &(wputp->candidate));
            dprintf("appending cand %p to nd %p\n",
                    &(wputp->depth), next_depthp);
            append(next_depthp, &(wputp->depth));
           
            ret = search_depth_vertical(sp,
                                        max_depth,
                                        start_depth + 1,
                                        OPPOSITE_COLOR(this_turn),
                                        &(wputp->next_depth),
                                        wputp);
            if (ret < 0) {
                ;
            }
            
        } else {
            /*
             * This turn is PASS.
             */
            printf("search_depth_vertical: depth=%d PASS\n", start_depth);
            retcode = -1;
#if 0
            append_passput(sp,
                           dp,
                           next_depthp,
                           this_turn);
#endif
        }
        
        
    } else {
        /*
         * bottom
         */
        return 0;
    }
    
    return retcode;
}


int think_level5(struct session *sp, struct put *p, int color)
{
    int retcode, pcount, ret, totalcand = 0, restcells, x, y;
    struct put *putp, *tmpput;
    struct queue *qp;
    int depth, this_turn, max_depth;
    struct depth *dp;

    tdprintf("think_level5: depth=%d (color:%d/turn:%d)\n",
           sp->cfg.depth, color, sp->turn);
    dprintf("num_put is %d\n", num_put);
    
    retcode = YES;

    restcells = sp->bd.xsize * sp->bd.ysize -
        get_occupied_cell_num(sp);
    dprintf("restcells = %d\n", restcells);
    
    tdprintf("think_level5 start. num_put is %d\n", num_put);
    /*
     * checek whether I have vertical searched candidate tree
     */
    if (IS_EMPTYQ(sp->player[sp->turn].depth)) {
        printf("This is the first vertical searched candidate tree build.\n");
        /*
         * search level=1 candidates
         */
        this_turn = color;
        depth = 2;
        ret = search_depth(sp, depth);
        if (ret == FAIL) {
            /*
             * FAIL
             */
            printf("think_level5: search_depth() failed.\n");
            exit(255);
            
        } else if ((depth == 1) && (ret == 0)) {
            /*
             * PASS
             */
#if 0
            tdprintf("think_level5: depth=1 is PASS!\n");
            putp = allocput();
            putp->color = color;
            putp->p.x = -1;
            putp->p.y = -1;
#endif
            dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
            append_passput(sp, dp, &(sp->player[sp->turn].next_depth), color);
#if 0
            append(&(dp->candidate), &(putp->candidate));
            append(&(sp->player[sp->turn].next_depth), &(putp->depth));
#endif
        }
        tdprintf("search_depth returns. num_put is %d\n", num_put);
            /*
             * search next level candidates
             */
        dp = Q_TO_DEPTH(GET_LAST_ELEMENT(sp->player[sp->turn].depth));
        qp = GET_TOP_ELEMENT(dp->candidate);
        while (!IS_ENDQ(qp, dp->candidate)) {
            putp = CANDIDATE_TO_PUT(qp);
            dprintf("calling search_depth_veritical for (%c%d)\n\n",
                   (int)putp->p.x + 'A', putp->p.y + 1);
            search_depth_vertical(sp,
                                  sp->cfg.depth,            /* max_depth */
                                  3,                        /* start depth */
                                  sp->turn,                 /* this_turn */
                                  &(putp->next_depth),      /* next_depth QH */
                                  putp);                    /* start  put */
            qp = qp->next;
        }
        
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
        
    } else {
        /*
         * 2nd stage and after
         *
         */
        struct put *prevputp;
        struct put *wputp;
        struct queue *qpsave;
        
        printf("I have pre-built candidate tree.\n");

        /*
         * search given put in the depth=1 candidate tree
         */
#if 0
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        while (!IS_ENDQ(qp, sp->player[sp->turn].depth)) {
            dp = Q_TO_DEPTH(qp);
            printf("dp=%p, IS_EMPTY(cand)=%d\n", dp, IS_EMPTYQ(dp->candidate));
            qp = qp->next;
        }
#endif        
        prevputp = NULL;
        wputp = MAIN_TO_PUT(GET_LAST_ELEMENT(sp->top));
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].next_depth);
        printf("is_endq of sp:player:next_depth is %d\n",
               IS_EMPTYQ(sp->player[sp->turn].next_depth));
        
        while (!IS_ENDQ(qp, sp->player[sp->turn].next_depth)) {
            qpsave = qp->next;
            putp = DEPTH_TO_PUT(qp);
            printf("color=%d (%c%d) %p\n", putp->color,
                    (int)putp->p.x + 'A', putp->p.y + 1, putp);
            if (IS_SAME_CELL(wputp, putp)) {
                printf("Found is previous put.\n");
                prevputp = putp;
            } else {
                /*
                 * clean up candidate trees below not used puts of
                 * opposite player
                 */
                cleanup_one_candidate_tree(putp);
                delete(&(putp->candidate));
                delete(&(putp->depth));
            }
            qp = qpsave;
        }

        putp = prevputp;
#ifdef DEBUG        
        dprintf("DEBUG %p\n", putp);
        printf("given put is %p (%c%d)\n", putp,
               (int)putp->p.x + 'A', putp->p.y + 1);
        
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
#endif
        /*
         * cleanup gotten put and its struct depth
         */
        dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));

        dprintf("dp=%p, candidate is %d\n", dp, IS_EMPTYQ(dp->candidate));
        
        delete(&(putp->candidate));
        delete(&(putp->depth));
        
        requeue_all(&(putp->next_depth), &(sp->player[sp->turn].next_depth));
        
        freeput(putp);
        delete(&(dp->q));
        freedepth(dp);
        
        /*
         * DEBUG
         */
        printf("\ncleaned up candidate tree\n");
#if 0
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);

        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        while (!IS_ENDQ(qp, sp->player[sp->turn].depth)) {
            dp = Q_TO_DEPTH(qp);
            printf("dp=%p, IS_EMPTY(cand)=%d\n", dp, IS_EMPTYQ(dp->candidate));
            qp = qp->next;
        }
#endif

        dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
        putp = CANDIDATE_TO_PUT(GET_TOP_ELEMENT(dp->candidate));
        
        printf("\nthink_level5: putp=%p (%c%d) \n",
               putp, putp->p.x + 'A', putp->p.y + 1);
        
        /*
         * rebuild possible candidates of depth=1
         */
        
        output(&(sp->bd));     /* DEBUG */
#if 0
        printf("think_level5: top dp->candidate is putp=%p (%c%d) %d emp:%d\n",
               putp, putp->p.x + 'A', putp->p.y + 1,
               putp->color, IS_EMPTYQ(dp->candidate)
               );

        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        while (!IS_ENDQ(qp, sp->player[sp->turn].depth)) {
            struct depth *wdp;
            wdp = Q_TO_DEPTH(qp);
            printf("dp=%p, cand=%d\n", wdp, IS_EMPTYQ(wdp->candidate));
            qp = qp->next;
        }
        printf("1: next_depth=%p, n:%p/p:%p\n",
               &(sp->player[sp->turn].next_depth),
               sp->player[sp->turn].next_depth.next,
               sp->player[sp->turn].next_depth.prev);
        printf("\n");
        
#endif
        have_candidates(sp,
                        &(sp->bd),
                        dp,
                        &(sp->player[sp->turn].next_depth),
                        putp,
                        color,
                        EVERY_CANDIDATE | EXCEPT_GIVEN);
        
        printf("\nrebuilt candidate tree\n");
#if 0        
        dprintf("2: next_depth=%p, n:%p/p:%p\n",
               &(sp->player[sp->turn].next_depth),
               sp->player[sp->turn].next_depth.next,
               sp->player[sp->turn].next_depth.prev);

        printf("\nafter have_candidates check!\n");
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        while (!IS_ENDQ(qp, sp->player[sp->turn].depth)) {
            struct queue *xqp;
            dp = Q_TO_DEPTH(qp);
            xqp = GET_TOP_ELEMENT(dp->candidate);
            printf("dp=%p, IS_EMPTY(cand)=%d\n", dp, IS_EMPTYQ(dp->candidate));
            while (!IS_ENDQ(xqp, dp->candidate)) {
                struct put *xputp;
                xputp = CANDIDATE_TO_PUT(xqp);
                printf("p=%p, (%c%d) %d\n", xputp,
                       xputp->p.x + 'A', xputp->p.y + 1, xputp->color);
                xqp = xqp->next;
            }
            qp = qp->next;
        }
#endif

        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);

        printf("\ncompleting depth=2 search \n");
        
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].depth);
        qp = qp->next;
        dp = Q_TO_DEPTH(qp);
        qp = GET_TOP_ELEMENT(sp->player[sp->turn].next_depth);
        while (!IS_ENDQ(qp, sp->player[sp->turn].next_depth)) {
            struct put *startputp;
            int search_mode;
            putp = DEPTH_TO_PUT(qp);
            printf("completing (%c%d)\n", putp->p.x + 'A', putp->p.y + 1);
            if (IS_EMPTYQ(putp->next_depth)) {
                startputp = NULL;
                search_mode = EVERY_CANDIDATE | START_ZERO;
            } else {
                startputp = DEPTH_TO_PUT(GET_TOP_ELEMENT(putp->next_depth));
                search_mode = EVERY_CANDIDATE | EXCEPT_GIVEN;
            }
            have_candidates(sp,
                            putp->bp,
                            dp,
                            &(putp->next_depth),
                            startputp,
                            OPPOSITE_COLOR(color),
                            search_mode);
            qp = qp->next;
        }
#if 1
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
        exit(255);
#endif
            /* level >=3 search */

        printf("\ncompleting depth>=3 search %d\n", sp->cfg.depth);
        
        dp = Q_TO_DEPTH(GET_LAST_ELEMENT(sp->player[sp->turn].depth));
        qp = GET_TOP_ELEMENT(dp->candidate);
        while (!IS_ENDQ(qp, dp->candidate)) {
            putp = CANDIDATE_TO_PUT(qp);
            dprintf("calling search_depth_veritical for (%c%d)\n\n",
                   (int)putp->p.x + 'A', putp->p.y + 1);
            search_depth_vertical(sp,
                                  sp->cfg.depth,            /* max_depth */
                                  3,                        /* start depth */
                                  sp->turn,                 /* this_turn */
                                  &(putp->next_depth),      /* next_depth QH */
                                  putp);                    /* start  put */
            qp = qp->next;
        }
        
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
        exit(255);
        
#if 0
            for (x = 0; x < sp->bd.xsize; x++) {
                for (y = 0; y < sp->bd.ysize; y++) {
                    if (CELL(sp->bd, x, y) != CELL(*(putp->bp), x, y))
                        printf("(%c%d) is not identical!!! sp/put=%d/%d\n",
                               x + 'A', y + 1,
                               CELL(sp->bd, x,y), CELL(*(putp->bp), x, y)
                               );
                }
            }
            printf("scan end.\n");
#endif

    }

    tdprintf("search_depth_vertical returns. num_put is %d\n", num_put);
    if (0) {
        print_candidate_tree(&(sp->player[sp->turn].next_depth), 1);
    }

    
    /*
     * evaluate candidate tree
     */
    tmpput = simple_strategy(sp, STRATEGY_CORNER_BORDER | STRATEGY_RANDOM);
    *p = *tmpput;
    
    /*
     * show my put
     */
    tdprintf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
           p->p.y + 1, p->gettable);
    /*
     * cleanup puts except the candidate tree under the put used above
     */
    dp = Q_TO_DEPTH(GET_TOP_ELEMENT(sp->player[sp->turn].depth));
    cleanup_top_depth(sp, dp, tmpput);

    printf("think_level5: num_put is %d\n", num_put);
    
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
        
    case 5:        
        retcode = think_level5(sp, p, color);
        break;

    default:
        printf("level=%d is not implemented yet.\n",
               sp->player[sp->turn].level);
        exit(255);
    }
    return retcode;
}


