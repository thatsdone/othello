/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"

#define NUM_DIRECTION 8
#define X 0
#define Y 1

int startvec[NUM_DIRECTION][2] =
{
    { 0, -1}, /* UP          */
    { 0,  1}, /* DOWN        */
    {-1,  0}, /* LEFT        */
    { 1,  0}, /* RIGHT       */
    {-1, -1}, /* UPPER_LEFT  */
    { 1, -1}, /* UPPER_RIGHT */
    {-1,  1}, /* LOWER_LEFT  */
    { 1,  1}  /* LOWER_RIGHT */
};


int unitvec[NUM_DIRECTION][2] =
{
    { 0,  1}, /* UP          */
    { 0, -1}, /* DOWN        */
    { 1,  0}, /* LEFT        */
    {-1,  0}, /* RIGHT       */
    { 1,  1}, /* UPPER_LEFT  */
    {-1,  1}, /* UPPER_RIGHT */
    { 1, -1}, /* LOWER_LEFT  */
    {-1, -1}  /* LOWER_RIGHT */
};

char *vecstr[] =
{
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "UPPER_LEFT",
    "UPPER_RIGHT",
    "LOWER_LEFT",
    "LOWER_RIGHT"
};

#define FOR_EACH_DIRECTION(dir) for(dir = 0; dir < NUM_DIRECTION; dir++)

int check_puttable(struct session *sp, struct put *p, int color)
{
    int dir, puttable = NO, ret = NO;

    p->gettable = 0;
/*    FOR_EACH_DIRECTION(dir) { */
    for(dir = 0; dir < NUM_DIRECTION; dir++) {
        
        puttable = check_puttable_dir(sp, p, color, dir);
        if (puttable == YES) {
            ret = YES;
            dprintf("puttalbe direction found: %s\n", vecstr[dir]);
        }
    }
    return ret;
}

int check_puttable_dir(struct session *sp, struct put *p, int color, int dir)
{
    int ret, retcode, neighbor;
    int x, y, px, py, dx, dy, ix, iy;
    int gettable = 0;
    int getwk;
    int range_check = 0;
    struct board *bp = &(sp->bd);

    /*
     *
     */
    dprintf("check_puttable_dir: checking direction: %s\n", vecstr[dir]);
    
    if (check_empty(sp, p, color) != YES) {
        dprintf("not empty\n");
    }
    retcode = NO;

    px = p->p.x;
    py = p->p.y;
    
    dx = -unitvec[dir][X];
    dy = -unitvec[dir][Y];
    
    ix = startvec[dir][X];
    iy = startvec[dir][Y];
#if 0
    if ((p->p.x == 6) && (p->p.y == 2)) {
        
    printf("check_neighbor %d\n", CHECK_NEIGHBOR(p, dir));
    printf("is_puttalble_range %d\n", IS_PUTTABLE_RANGE(p->p.x, p->p.y, bp));
    printf("looking cell is (x,y)=(%d,%d) color is %d, mine (%d))\n",
           px + ix, py + iy, CELL(*bp, px + ix, py + iy), color);
    }
#endif

    switch (dir) {
    case UP:
        range_check = (py >= MIN_PUTTABLE_OFFSET);
        break;
    case DOWN:
        range_check = (py <= MAX_Y - MIN_PUTTABLE_OFFSET);
        break;
    case LEFT:
        range_check = (py >= MIN_PUTTABLE_OFFSET);
        break;
    case RIGHT:
        range_check = (px <= MAX_X - MIN_PUTTABLE_OFFSET);
        break;
    case UPPER_LEFT:
        range_check = (px >= MIN_PUTTABLE_OFFSET) && (py >= MIN_PUTTABLE_OFFSET);
        break;
    case UPPER_RIGHT:
        range_check = (px <= MAX_X - MIN_PUTTABLE_OFFSET) && (py >= MIN_PUTTABLE_OFFSET);
        break;        
    case LOWER_LEFT:
        range_check = (px >= MIN_PUTTABLE_OFFSET) && (py <= MAX_Y - MIN_PUTTABLE_OFFSET);
        break;
 
    case LOWER_RIGHT:
        range_check = (px <= MAX_X - MIN_PUTTABLE_OFFSET) && (py <= MAX_Y - MIN_PUTTABLE_OFFSET);
        break;
    }
    dprintf("dir is %d, range_check is %d\n", dir, range_check);
    
    
    if (CHECK_NEIGHBOR(p, dir) &&
        range_check &&
        (CELL(*bp, px + ix, py + iy) == OPPOSITE_COLOR(p->color))) {
        dprintf("hagehage %d\n", IS_SCANNABLE_RANGE(px+ix, py+iy, bp));
        
        getwk = 1;
        for (x = px + ix, y = py + iy;
             IS_SCANNABLE_RANGE(x, y, bp);
             x += dx, y += dy) {
            dprintf("loop: x = %d, y = %d, color = %d\n",
                    x, y, CELL(*bp, x, y));
            if (CELL(*bp, x, y) == p->color) {
                dprintf("found puttable line: %s\n", vecstr[dir]);
                SET_CANGET(p, dir);
                retcode = YES;
                break;
            } else if (IS_EMPTY_CELL(*bp, x, y)) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        if (retcode == YES) {
            gettable += getwk;
        }
    } else {
        dprintf("direction %s is not gettable line.\n", vecstr[dir]);
    }
    p->gettable += gettable;
    
    return retcode;
    
}



int check_empty(struct session *sp, struct put *p, int color)
{
    int x, y, ret = YES;

    x = p->p.x;
    y = p->p.y;
    
    if (!IS_EMPTY_CELL(sp->bd, x, y))
        return NO;

    /*
     * left
     */
    if ((x > 0) && !IS_EMPTY_CELL(sp->bd, x - 1, y)) {
        dprintf("left is not empty\n");
        p->neighbor.b.left = YES;
        ret = YES;
    }
    /*
     * right
     */
    if ((x < MAX_X) && !IS_EMPTY_CELL(sp->bd, x + 1, y)) {
        dprintf("right is not empty\n");
        p->neighbor.b.right = YES;
        ret = YES;
    }
    /*
     * up
     */
    if ((y > 0) && !IS_EMPTY_CELL(sp->bd, x, y - 1)) {
        dprintf("up is not empty\n");
        p->neighbor.b.up = YES;
        ret = YES;        
    }
    /*
     * down
     */
    if ((y < MAX_Y) && !IS_EMPTY_CELL(sp->bd, x, y + 1)) {
        dprintf("down is not empty\n");
        p->neighbor.b.down = YES;
        ret = YES;        
    }
    /*
     * upper left
     */
    if ((x > 0) && (y > 0) && !IS_EMPTY_CELL(sp->bd, x - 1, y - 1)) {
        dprintf("upper left is not empty\n");
        p->neighbor.b.upper_left = YES;
        ret = YES;        
    }
    /*
     * upper right
     */
    if ((x < MAX_X) && (y > 0) && !IS_EMPTY_CELL(sp->bd, x + 1, y - 1)) {
        dprintf("upper right is not empty\n");
        p->neighbor.b.upper_right = YES;
        ret = YES;        

    }
    /*
     * lower left
     */
    if ((x > 0) && (y < MAX_Y) &&
        !IS_EMPTY_CELL(sp->bd, x - 1, y + 1)) {
        dprintf("lower left is not empty\n");
        p->neighbor.b.lower_left = YES;        
        ret = YES;        
    }
    /*
     * lower right
     */
    if ((x < MAX_X) && (y < MAX_Y) && !IS_EMPTY_CELL(sp->bd,x + 1, y + 1)) {
        dprintf("lower right is not empty\n");
        p->neighbor.b.lower_right = YES;
        ret = YES;        
    }
    dprintf("(%d,%d) neighbor = %08x\n", x, y, p->neighbor.i);
    
    return ret;
        
}

int process_put(struct session *sp, struct put *p, int color)
{
    int x, y;
    struct board *bp = &(sp->bd);
    
    dprintf("process_put\n");
    /*
     * up
     */
    if (CHECK_CANGET(p, UP)) {
        for (y = p->p.y - 1; y > MIN_Y; y--) {
            if (CELL(*bp, p->p.x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, p->p.x, y) = color;
            } else {
                break;
            }
        }
    }
    /*
     * down
     */
    if (CHECK_CANGET(p, DOWN)) {
        for (y = p->p.y + 1; y < MAX_Y; y++) {
            if (CELL(*bp, p->p.x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, p->p.x, y) = color;
            } else {
                break;
            }
        }
    }
    /*
     * left
     */
    if (CHECK_CANGET(p, LEFT)) {
        for (x = p->p.x - 1; x > MIN_X; x--) {
            if (CELL(*bp, x, p->p.y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, p->p.y) = color;
            } else {
                break;
            }
        }
    }
    
    /*
     * right
     */
    if (CHECK_CANGET(p, RIGHT)) {
        for (x = p->p.x + 1; x < MAX_X; x++) {
            if (CELL(*bp, x, p->p.y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, p->p.y) = color;
            } else {
                break;
            }
        }
    }
    /*
     * upper left
     */
    if (CHECK_CANGET(p, UPPER_LEFT)) {
        for (x = p->p.x - 1, y = p->p.y - 1;
             (x > MIN_X && y > MIN_Y); x--, y--) {
            if (CELL(*bp, x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, y) = color;
            } else {
                break;
            }
        }
    }
    /*
     * upper right
     */
    if (CHECK_CANGET(p, UPPER_RIGHT)) {
        for (x = p->p.x + 1, y = p->p.y - 1;
             (x < MAX_X && y > MIN_Y); x++, y--) {
            if (CELL(*bp, x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, y) = color;
            } else {
                break;
            }
        }
    }
    /*
     * lower left
     */
    if (CHECK_CANGET(p, LOWER_LEFT)) {
        for (x = p->p.x - 1, y = p->p.y + 1;
             (x > MIN_X && y < MAX_Y); x--, y++) {
            if (CELL(*bp, x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, y) = color;
            } else {
                break;
            }
        }
    }
    
    /*
     * lower right
     */
    if (CHECK_CANGET(p, LOWER_RIGHT)) {
        for (x = p->p.x + 1, y = p->p.y + 1;
             (x < MAX_X && y < MAX_Y); x++, y++) {
            if (CELL(*bp, x, y) == OPPOSITE_COLOR(color)) {
                CELL(*bp, x, y) = color;
            } else {
                break;
            }
        }
    }
    
    return 0;
}






