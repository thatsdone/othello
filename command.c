/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"


extern struct queue top;

void command_show(void)
{
    
    struct queue *q;
    struct put *p;
    int i = 1;
            
    q = top.next;
    while (q != &top) {
        p = (struct put *)q;
        printf("count=%2d, color=%2d, q=%p, (x,y)=(%d,%d)\n",
               i, p->color, q, p->p.x, p->p.y
               );
        q = q->next;
        i++;
    }
}

