/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"


extern struct queue top;

void command_show(struct session *sp)
{
    struct queue *q;
    struct put *p;
    int i = 1;
    printf("command_show\n");
    
    
    q = GET_TOP_ELEMENT(sp->top);
    while (!IS_ENDQ(q, sp->top)) {
        p = (struct put *)q;
        printf("count=%2d, color=%2d, q=%p, (x,y)=(%d,%d) n:%08x, c:%08x\n",
               i, p->color, q, p->p.x, p->p.y, p->neighbor.i, p->canget.i
               );
        q = q->next;
        i++;
    }
}

