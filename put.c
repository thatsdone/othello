/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "othello.h"


int check_empty(struct board *bp, struct put *p, int color)
{
    int x, y, ret = YES;

    x = p->p.x;
    y = p->p.y;
    
    if (bp->b[x][y] != EMPTY)
        return NO;

    /*
     * left
     */
    if ((x > 0) && (bp->b[x - 1][y] != EMPTY)) {
        dprintf("left is not empty\n");
        p->neighbor.left = YES;
        ret = YES;
    }
    /*
     * right
     */
    if ((x <= BOARDSIZE - 1) &&
        (bp->b[x + 1][y] != EMPTY)) {
        dprintf("right is not empty\n");
        p->neighbor.right = YES;
        ret = YES;
    }
    /*
     * up
     */
    if ((y > 0) && (bp->b[x][y - 1] != EMPTY)) {
        dprintf("up is not empty\n");
        p->neighbor.up = YES;
        ret = YES;        
    }
    /*
     * down
     */
    if ((y <= BOARDSIZE - 1) &&
               (bp->b[x][y + 1] != EMPTY)) {
        dprintf("down is not empty\n");
        p->neighbor.down = YES;
        ret = YES;        
    }
    /*
     * upper left
     */
    if ((x > 0) && (y > 0) &&
        (bp->b[x - 1][y - 1] != EMPTY)) {
        dprintf("upper left is not empty\n");
        p->neighbor.upper_left = YES;
        ret = YES;        
    }
    /*
     * upper right
     */
    if ((x <= BOARDSIZE - 1) && (y > 0) &&
        (bp->b[x + 1][y - 1] != EMPTY)) {
        dprintf("upper right is not empty\n");
        p->neighbor.upper_right = YES;
        ret = YES;        

    }
    /*
     * lower left
     */
    if ((x > 0) && (y <= BOARDSIZE - 1) &&
        (bp->b[x - 1][y + 1] != EMPTY)) {
        dprintf("lower left is not empty\n");
        p->neighbor.lower_left = YES;        
        ret = YES;        
    }
    /*
     * lower right
     */
    if ((x <= BOARDSIZE - 1) && (x <= BOARDSIZE - 1) &&
        (bp->b[x + 1][y + 1] != EMPTY)) {
        dprintf("lower right is not empty\n");
        p->neighbor.lower_right = YES;
        ret = YES;        
    }

    return ret;
        
}

int process_put(struct board *bp, struct put *p, int color)
{
    int x, y;
    dprintf("process_put\n");
    /*
     * up
     */
    if (p->canget.up & YES) {
        for (y = p->p.y - 1; y > MIN_Y; y--) {
            dprintf("x=%d/y=%d b-color=%d\n", p->p.x, y, bp->b[p->p.x][y]);
            if (bp->b[p->p.x][y] == OPPOSITE_COLOR(color)) {
                bp->b[p->p.x][y] = color;
            } else {
                break;
            }
        }
    }
    /*
     * down
     */
    if (p->canget.down & YES) {
        for (y = p->p.y + 1; y < MAX_Y; y++) {
            dprintf("x=%d/y=%d b-color=%d\n", p->p.x, y, bp->b[p->p.x][y]);
            if (bp->b[p->p.x][y] == OPPOSITE_COLOR(color)) {
                bp->b[p->p.x][y] = color;
            } else {
                break;
            }
        }
    }
    /*
     * left
     */
    if (p->canget.left & YES) {
        for (x = p->p.x - 1; x > MIN_X; x--) {
            dprintf("x=%d/y=%d b-color=%d\n", x, p->p.y, bp->b[x][p->p.y]);
            if (bp->b[x][p->p.y] == OPPOSITE_COLOR(color)) {
                bp->b[x][p->p.y] = color;
            } else {
                break;
            }
        }
    }
    
    /*
     * right
     */
    if (p->canget.right & YES) {
        for (x = p->p.x + 1; x < MAX_X; x++) {
            dprintf("x=%d/y=%d b-color=%d\n", x, p->p.y, bp->b[x][p->p.y]);
            if (bp->b[x][p->p.y] == OPPOSITE_COLOR(color)) {
                bp->b[x][p->p.y] = color;
            } else {
                break;
            }
        }
    }
    /*
     * upper left
     */
    if (p->canget.upper_left & YES) {
        for (x = p->p.x - 1, y = p->p.y - 1;
             (x > MIN_X && y > MIN_Y); x--, y--) {
            dprintf("x=%d/y=%d b-color=%d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == OPPOSITE_COLOR(color)) {
                bp->b[x][y] = color;
            } else {
                break;
            }
        }
    }
    /*
     * upper right
     */
    if (p->canget.upper_right & YES) {
        for (x = p->p.x + 1, y = p->p.y - 1;
             (x < MAX_X && y > MIN_Y); x++, y--) {
            dprintf("x=%d/y=%d b-color=%d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == OPPOSITE_COLOR(color)) {
                bp->b[x][y] = color;
            } else {
                break;
            }
        }
    }
    /*
     * lower left
     */
    if (p->canget.lower_left & YES) {
        for (x = p->p.x - 1, y = p->p.y + 1;
             (x > MIN_X && y < MAX_Y); x--, y++) {
            dprintf("x=%d/y=%d b-color=%d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == OPPOSITE_COLOR(color)) {
                bp->b[x][y] = color;
            } else {
                break;
            }
        }
    }
    
    /*
     * lower right
     */
    if (p->canget.lower_right & YES) {
        for (x = p->p.x + 1, y = p->p.y + 1;
             (x < MAX_X && y < MAX_Y); x++, y++) {
            dprintf("x=%d/y=%d b-color=%d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == OPPOSITE_COLOR(color)) {
                bp->b[x][y] = color;
            } else {
                break;
            }
        }
    }
    
    return 0;
}

   
int check_puttable(struct board *bp, struct put *p, int color)
{
    int ret, retcode = NO;
    int x, y, px, py;
    int puttable = NO;
    int gettable = 0;
    int getwk;
    
    dprintf("check_puttable: x=%d/y=%d, color=%d, %p\n",
            p->p.x, p->p.y, color, p);
    ret = check_empty(bp, p, color);
    dprintf("check_empty returns %d\n", ret);
    if (ret == NO) {
        return retcode;
    }
    
    px = p->p.x;
    py = p->p.y;
    dprintf("p-color:%d, o-color:%d, b-color(up):%d\n",
           color, OPPOSITE_COLOR(color),
           bp->b[px][py - 1]);
    /*
     * up
     */
    getwk = 1;
    dprintf("check_puttable: checking up\n");
    if ((p->neighbor.up & YES) && (py >= MIN_PUTTABLE_OFFSET) &&
        (bp->b[px][py - 1] == OPPOSITE_COLOR(color))) {
        for (y = py - PUT_CHECK_OFFSET; y >= MIN_Y; y--) {
            dprintf("loop: y = %d, color = %d\n", y, bp->b[px][y]);
            if (bp->b[px][y] == color) {
                dprintf("found puttable line: up\n");
                p->canget.up = YES;
                retcode = YES;
                break;
            } else if (bp->b[px][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Up is not gettable line.\n");
        dprintf("neighbor.up = %d\n", p->neighbor.up & 1);
    }
    /*
     * down
     */
    getwk = 1;
    dprintf("check_puttable: checking down\n");
    if ((p->neighbor.down & YES) && (py <= MAX_Y - MIN_PUTTABLE_OFFSET) &&
        (bp->b[px][py + 1] == OPPOSITE_COLOR(color))) {
        for (y = py + PUT_CHECK_OFFSET; y <= MAX_Y; y++) {
            dprintf("loop: y = %d, color = %d\n", y, bp->b[px][y]);
            if (bp->b[px][y] == color) {
                dprintf("found puttable line: down\n");
                p->canget.down = YES;
                retcode = YES;
                break;
            } else if (bp->b[px][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Down is not gettable line.\n");
        dprintf("neighbor.down = %d\n", p->neighbor.down & 1);
    }
    /*
     * left
     */
    getwk = 1;
    dprintf("check_puttable: checking left\n");
    if ((p->neighbor.left & YES) && (px >= MIN_PUTTABLE_OFFSET) &&
        (bp->b[px - 1][py] == OPPOSITE_COLOR(color))) {
        for (x = px - PUT_CHECK_OFFSET; x >= MIN_X; x--) {
            dprintf("loop: x = %d, color = %d\n", x, bp->b[x][py]);
            if (bp->b[x][py] == color) {
                dprintf("found puttable line: left\n");
                p->canget.left = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][py] == EMPTY) {
                getwk = 0;                
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Left is not gettable line.\n");
        dprintf("neighbor.left = %d\n", p->neighbor.left & 1);
    }
    
    /*
     * right
     */
    getwk = 1;
    dprintf("check_puttable: checking right\n");
    if ((p->neighbor.right & YES) && (px <= MAX_X - MIN_PUTTABLE_OFFSET) &&
        (bp->b[px + 1][py] == OPPOSITE_COLOR(color))) {
        for (x = px + PUT_CHECK_OFFSET; x <= MAX_X; x++) {
            dprintf("loop: x = %d, color = %d\n", x, bp->b[x][py]);
            if (bp->b[x][py] == color) {
                dprintf("found puttable line: right\n");
                p->canget.right = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][py] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Right is not gettable line.\n");
        dprintf("neighbor.down = %d\n", p->neighbor.down & 1);
    }
    /*
     * upper left
     */
    getwk = 1;
    dprintf("check_puttable: checking upper left\n");
    if ((p->neighbor.upper_left & YES) &&
        (px >= MIN_PUTTABLE_OFFSET) &&        
        (py >= MIN_PUTTABLE_OFFSET) &&
        (bp->b[px - 1][py - 1] == OPPOSITE_COLOR(color))) {
        for (y = py - PUT_CHECK_OFFSET, x = px - PUT_CHECK_OFFSET;
             (y >= MIN_Y && x >= MIN_X); y--, x--) {
            dprintf("loop: x = %d, y = %d, color = %d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == color) {
                dprintf("found puttable line: upper left\n");
                p->canget.upper_left = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Upper Left is not gettable line.\n");
        dprintf("neighbor.upper_left = %d\n", p->neighbor.upper_left & 1);
    }
    /*
     * upper right
     */
    getwk = 1;
    dprintf("check_puttable: checking upper right\n");
    if ((p->neighbor.upper_right & YES) &&
        (px <= MAX_X - MIN_PUTTABLE_OFFSET) &&
        (py >= MIN_PUTTABLE_OFFSET) &&
        (bp->b[px + 1][py - 1] == OPPOSITE_COLOR(color))) {
        for (y = py - PUT_CHECK_OFFSET,x = px + PUT_CHECK_OFFSET;
             (y >= MIN_Y && x <= MAX_X); y--, x++) {
            dprintf("loop: x = %d, y = %d, color = %d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == color) {
                dprintf("found puttable line: upper right\n");
                p->canget.upper_right = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Upper Right is not gettable line.\n");
        dprintf("neighbor.upper_right = %d\n", p->neighbor.upper_right & 1);
    }
    
    /*
     * lower left
     */
    getwk = 1;
    dprintf("check_puttable: checking lower left\n");
    if ((p->neighbor.lower_left & YES) &&
        (px >= MIN_PUTTABLE_OFFSET) &&
        (py <= MAX_Y - MIN_PUTTABLE_OFFSET) &&
        (bp->b[px - 1][py + 1] == OPPOSITE_COLOR(color))) {
        for (x = px - PUT_CHECK_OFFSET, y = py + PUT_CHECK_OFFSET;
             (x >= MIN_X && y <= MAX_Y); x--, y++) {
            dprintf("loop: x = %d, y = %d, color = %d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == color) {
                dprintf("found puttable line: upper right\n");
                p->canget.lower_left = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Lower Left is not gettable line.\n");
        dprintf("neighbor.upper_right = %d\n", p->neighbor.upper_right & 1);
    }
    
    /*
     * lower right
     */
    getwk = 1;
    dprintf("check_puttable: checking lower right\n");
    if ((p->neighbor.lower_right & YES) &&
        (px <= MAX_X - MIN_PUTTABLE_OFFSET) &&
        (py <= MAX_Y - MIN_PUTTABLE_OFFSET) &&
        (bp->b[px + 1][py + 1] == OPPOSITE_COLOR(color))) {
        for (y = py + PUT_CHECK_OFFSET, x = px + PUT_CHECK_OFFSET;
             (y <= MAX_Y && x <= MAX_X); y++, x++) {
            dprintf("loop: x = %d, y = %d, color = %d\n", x, y, bp->b[x][y]);
            if (bp->b[x][y] == color) {
                dprintf("found puttable line: lower right\n");
                p->canget.lower_right = YES;
                retcode = YES;
                break;
            } else if (bp->b[x][y] == EMPTY) {
                getwk = 0;
                break;
            }
            getwk++;
        }
        gettable += getwk;
    } else {
        dprintf("Lower Right is not gettable line.\n");
        dprintf("neighbor.lower_right = %d\n", p->neighbor.lower_right & 1);
    }

    p->gettable = gettable;
    
    return retcode;
}

