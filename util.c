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
    struct queue *qp;
    
    qp = GET_TOP_ELEMENT(*from);

    delete(from);

    delete(to);
    
    to->next = qp;
    to->prev = qp->prev;
    qp->prev->next = to;
    qp->prev = to;
    
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
#if 0
        INITQ(p->depth);
#endif
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


