/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"


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

void initdepth(struct depth *dp, int depth)
{
    memset(dp, 0x00, sizeof(struct depth));
    INITQ(dp->q);
    dp->depth = depth;
    INITQ(dp->candidate);
    INITQ(dp->next_depth);
    return;
}

struct depth *allocdepth(void)
{
    struct depth *dp;
    
    dp = (struct depth *)malloc(sizeof(struct depth));
    return dp;
}

void freedepth(struct depth *dp)
{
    free((void *)dp);
    return;
}

