/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"

int orand(int max)
{
    return (int) ((double)max * (random() / (double)(RAND_MAX)));
}



void append(struct queue *head, struct queue *target)
{
    dprintf("append: head=%p, target=%p\n", head, target);
    dprintf("append: head->next=%p, head->prev=%p\n", head->next,head->prev);
    
    target->prev = head->prev;
    target->next = head;
    
    head->prev->next = target;
    head->prev = target;
    dprintf("append: head->next=%p, head->prev=%p\n", head->next,head->prev);
    dprintf("append: target->next=%p, target->prev=%p\n", target->next,target->prev);
}

void push(struct queue *head, struct queue *target)
{
    dprintf("push: head=%p, target=%p\n", head, target);
    dprintf("push: head->next=%p, head->prev=%p\n", head->next,head->prev);
    
    target->prev = head;
    target->next = head->next;
    
    head->next->prev = target;
    head->next = target;
    dprintf("push: head->next=%p, head->prev=%p\n", head->next,head->prev);
    dprintf("push: target->next=%p, target->prev=%p\n", target->next,target->prev);
}

void delete(struct queue *target)
{
    struct queue *save;
    
    dprintf("delete: target=%p\n", target);
    dprintf("delete: target->next=%p, target->prev=%p\n", target->next,target->prev);
    
    target->prev->next = target->next;
    target->next->prev = target->prev;
    
    target->next = target;
    target->prev = target;
    
}


void requeue_all(struct queue *from, struct queue *to)
{
    struct queue *topqp, *lastqp;


    if (!IS_EMPTYQ(*to)) {
        struct put *putp;
        printf("requeue_all: queue 'to' is not EMPTY!!!\n");
        putp = DEPTH_TO_PUT(GET_TOP_ELEMENT(*to));
        printf("requeue_all: (%c%d) color=%d\n", (int)putp->p.x + 'A',
               putp->p.y + 1, putp->color);
            
    }
    
    topqp  = GET_TOP_ELEMENT(*from);
    lastqp = GET_LAST_ELEMENT(*from);
    
    
    to->next = topqp;
    to->prev = lastqp;
    topqp->prev = to;
    lastqp->next = to;
    
    dprintf("requeue_all: to->next is %p\n", topqp);
    dprintf("requeue_all: to->prev is %p\n", lastqp);
    dprintf("requeue_all: topqp->prev is %p\n", to);
    dprintf("requeue_all: lastqp->next is %p\n", to);
    
    INITQ(*from);
    
    return;
}





int num_put = 0;

struct put *allocput(void)
{
    struct put *p;
    p = (struct put *)malloc(sizeof(struct put));
    memset(p, 0x00, sizeof(struct put));
    if (p != NULL) {
        memset(p, 0x00, sizeof(struct put));
        INITQ(p->main);
        INITQ(p->candidate);
        INITQ(p->next_depth);
    }
    num_put++;
    return p;
}


void freeput(struct put *p)
{
    dprintf("freeput: %p\n", p);
    
    free((void *)p);
    num_put--;
}

void initput(struct put *p)
{
    if (p != NULL) {
        memset(p, 0x00, sizeof(struct put));
        INITQ(p->main);
        INITQ(p->candidate);
        INITQ(p->depth);
        INITQ(p->next_depth);        
    }
    return;
}

struct depth *allocdepth(void)
{
    struct depth *dp;
    
    dp = (struct depth *)malloc(sizeof(struct depth));
    return dp;
}

void initdepth(struct depth *dp, int depth)
{
    memset(dp, 0x00, sizeof(struct depth));
    INITQ(dp->q);
#if 0
    dp->depth = depth;
#endif
    INITQ(dp->candidate);
    return;
}

void freedepth(struct depth *dp)
{
    free((void *)dp);
    return;
}
#if 0
void show_put(struct put *p, int show_gettable) 
{
    if (show_getttable) {
        printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
               p->p.y + 1, p->gettable);
    } else {
        printf("use (%c%d), gettable=%d\n", (int)p->p.x + 'A',
               p->p.y + 1, p->gettable);
    }
    return;
}
#endif


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


void dump_config(struct config *cfp)
{
    printf("cfg.boardsize is %d\n", cfp->boardsize);
    printf("cfg.level is %d\n",     cfp->level);
    printf("cfg.mode is %d\n", cfp->mode);
    printf("cfg.serve_first is %d\n", cfp->serve_first);
    printf("cfg.debug is %d\n", cfp->debug_level);
    printf("cfg.remote_host is %s\n", cfp->remote_host);
    printf("cfg.remote_port is %d\n", cfp->remote_port);
    
    printf("cfg.depth is %d\n", cfp->depth);
    
    printf("cfg.opt_b is %d\n", cfp->opt_b);
    printf("cfg.opt_h is %d\n", cfp->opt_h);
    printf("cfg.opt_p is %d\n", cfp->opt_p);
    printf("cfg.opt_l is %d\n", cfp->opt_l);
    printf("cfg.opt_m is %d\n", cfp->opt_m);
    printf("cfg.opt_f is %d\n", cfp->opt_f);
    printf("cfg.opt_d is %d\n", cfp->opt_d);
    printf("cfg.opt_0 is %d\n", cfp->opt_0);
    printf("cfg.opt_1 is %d\n", cfp->opt_1);
    printf("cfg.opt_3 is %d\n", cfp->opt_3);
    printf("cfg.opt_o is %d\n", cfp->opt_o);
    printf("cfg.opt_v is %d\n", cfp->opt_v);
    printf("cfg.opt_D is %d\n", cfp->opt_D);

    return;
    
}

void dump_player(struct player *p, int pl)
{
    printf("player[%d].type is %d\n", pl, p[pl].type);
    printf("player[%d].level is %d\n", pl, p[pl].level);
    printf("player[%d].candidate.next is %p\n", pl, p[pl].candidate.next);
    printf("player[%d].candidate.prev is %p\n", pl, p[pl].candidate.prev);
    printf("player[%d].next_depth.next is %p\n", pl, p[pl].next_depth.next);
    printf("player[%d].next_depth.prev is %p\n", pl, p[pl].next_depth.prev);
    printf("player[%d].depth.next is %p\n", pl, p[pl].depth.next);
    printf("player[%d].depth.prev is %p\n", pl, p[pl].depth.prev);
    
    printf("player[%d].num_candidate is %d\n", pl, p[pl].num_candidate);

    return;
}


void dump_put(struct put *putp)
{
    printf("putp is %p\n", putp);
    
    printf("putp->neighbor is %08x\n", putp->neighbor.i);
    printf("putp->canget   is %08x\n", putp->canget.i);
    printf("putp->gettable is %d\n", putp->gettable);

    printf("putp->next_depth of IS_EMPTYQ(%d)\n", IS_EMPTYQ(putp->next_depth));
    printf("putp->next_depth.next is %p\n", putp->next_depth.next);
    printf("putp->next_depth.prev is %p\n", putp->next_depth.prev);
    
    printf("putp->depth of IS_EMPTYQ(%d)\n", IS_EMPTYQ(putp->depth));
    printf("putp->depth.next is %p\n", putp->depth.next);
    printf("putp->depth.prev is %p\n", putp->depth.prev);

    printf("putp->bp is %p\n", putp->bp);
    /* output(putp->bp); */
    printf("putp->up is %p\n", putp->up);
    printf("putp->dp is %p\n", putp->dp);
    
    printf("putp->black  is %d\n", putp->black);
    printf("putp->whilte is %d\n", putp->white);
    return;
}



void dump_data(struct session *sp)
{
    /*
     * struct session 
     */
    /* cfg */
    output(&(sp->bd));
    command_show(sp);
    printf("sp->counter is %d\n", sp->counter);
    printf("sp->turn is %d\n", sp->turn);
    printf("sp->was_pass is %d\n", sp->was_pass);
    printf("sp->is_end is %d\n", sp->is_end);
    printf("sp->buf is %p\n", sp->buf);
    
    /*
     * struct config
     */
    dump_config(&(sp->cfg));
    
    /*
     * struct player
     */
    {
        int p;
        for (p = 0; p < NUM_PLAYER; p++) {
            dump_player(sp->player, p);
        }
    }

    {
        int p;
        for (p = 0; p < NUM_PLAYER; p++) {
            printf("\nprint_candidate_tree for player[%d]\n", p);
            print_candidate_tree(&(sp->player[p].next_depth), 1);
        }
    }
    {
        int p;
        struct queue *dqp, *pqp;
        struct depth *dp;
        struct put *putp;
        
        for (p = 0; p < NUM_PLAYER; p++) {
            dqp = GET_TOP_ELEMENT(sp->player[p].depth);
            while (!IS_ENDQ(dqp, sp->player[p].depth)) {
                dp = Q_TO_DEPTH(dqp);
                
                printf("dp=%p\n", dp);
                printf("dp->candiate.next is %p\n", dp->candidate.next);
                printf("dp->candiate.prev is %p\n", dp->candidate.prev);
                printf("dp->num_cand is %p\n", dp->num_cand);
                
                pqp = GET_TOP_ELEMENT(dp->candidate);
                while (!IS_ENDQ(pqp, dp->candidate)) {
                    putp = CANDIDATE_TO_PUT(pqp);
                    
                    dump_put(putp);
                    
                    pqp = pqp->next;
                }
                dqp = dqp->next;
            }
        }
    }
    
    
    return;
    
}


