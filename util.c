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
    printf("push: head=%p, target=%p\n", head, target);
    printf("push: head->next=%p, head->prev=%p\n", head->next,head->prev);
    
    target->prev = head;
    target->next = head->next;
    
    head->next->prev = target;
    head->next = target;
    printf("push: head->next=%p, head->prev=%p\n", head->next,head->prev);
    printf("push: target->next=%p, target->prev=%p\n", target->next,target->prev);
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


struct put *allocput(void)
{
    struct put *p;
    p = (struct put *)malloc(sizeof(struct put));
    if (p != NULL) {
        memset(p, 0x00, sizeof(struct put));
        p->main.next = (struct queue *)&(p->main);
        p->main.prev = (struct queue *)&(p->main);
        p->candidate.next = (struct queue *)&(p->candidate);
        p->candidate.prev = (struct queue *)&(p->candidate);
    }
    return p;
}


void freeput(struct put *p)
{
    free((void *)p);
}

