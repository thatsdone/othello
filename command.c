/*
 *   $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#include "othello.h"

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

int command_status(struct session *sp)
{
    printf("boardsize: (x=%d, y=%d)\n", sp->bd.xsize, sp->bd.ysize);
    printf("player0(BLACK)=%s(level=%d), player1(WHITE)=%s(level=%d)",
           player_type(sp->player[PLAYER_FIRST].type),
           sp->player[PLAYER_FIRST].level,
           player_type(sp->player[PLAYER_SECOND].type),
           sp->player[PLAYER_SECOND].level);
    printf("\n");
    
}

int command_pass(struct session *sp, struct put *putp)
{
    int x, y, ret;
    struct put *wputp;

    wputp = allocput();
    
    for (x = 0; x < sp->bd.xsize; x++) {
        for (y = 0; y < sp->bd.ysize; y++) {
            initput(wputp);
            wputp->p.x = x;
            wputp->p.y = y;
            wputp->color = OPPOSITE_COLOR(sp->turn);
            ret = check_puttable(sp, &(sp->bd), wputp,
                                 OPPOSITE_COLOR(sp->turn));
            if (ret == YES) {
                printf("Illegal PASS!\n");
                freeput(wputp);
                return NO;
            }
        }
    }
    
    append(&(sp->top), &(putp->main));
    if (sp->was_pass == YES) {
        finalize(&(sp->bd));
        
    } else {
        putp->p.x = -1;
        putp->p.x = -1;
        append(&(sp->top), &(putp->main));
        FLIP_COLOR(sp);
        sp->was_pass = YES;
    }
    
    freeput(wputp);
    return YES;
}

int command_save(struct session *sp)
{
    char *alphastr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPWRSTUVWXYZ";
    char *c;
    char *filename;
    int fd;
    int i;
    FILE *fp;
    struct queue *qp;
    struct put *putp;
    time_t t;
    char timestr[256];
    struct tm *tmp;
    
    c = (char *)(sp->buf + 4);
    while (*c == ' ') {
        c++;
    }
    filename = c;
    dprintf("filename is %s\n", filename);
#define CFILE_MODE  (S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH)
#define CFILE_FLAGS (O_RDWR | O_CREAT | O_EXCL)
    fd = open(filename, CFILE_FLAGS, CFILE_MODE);
    if (fd < 0) {
        printf("open failed (%d: %s)\n", errno, strerror(errno));
        return NO;
    }
    dprintf("fd is %d\n", fd);
    
    fp = fdopen(fd, "r+");
    if (fp == NULL) {
        printf("fdopen failed.(%d)\n", errno);
        printf("");
        close(fd);
        return NO;
    }

    fprintf(fp, "(;\n");
        /* File Format */
    fprintf(fp, "FF[4]\n");
        /* Game type  */
    fprintf(fp, "GM[2]\n");    
        /* board size */
    fprintf(fp, "SZ[%d]\n", sp->cfg.boardsize);
        /* date */
    fprintf(fp, "GN[marc othello %s]\n", VERSION);
        /* date */
    t = time(&t);
    strcpy(timestr, ctime(&t));
    tmp = localtime(&t);
    
    timestr[strlen(timestr) - 1] = (char)0x00;
    
    fprintf(fp, "DT[%s]\n", timestr);

    qp = GET_TOP_ELEMENT(sp->top);
    
    while (qp !=  &(sp->top)) {
        putp = MAIN_TO_PUT(qp);
        if (putp->color == BLACK) {
            fprintf(fp, ";B[");
        } else {
            fprintf(fp, ";W[");
        }
        
        if (putp->p.x != -1 ){
            fprintf(fp, "%c%c]\n",
                    (int)*(alphastr + putp->p.x),
                    (int)*(alphastr + putp->p.y));
        } else {
                /* pass */
            fprintf(fp, "]\n");
        }
        qp = qp->next;
    }
    
    fprintf(fp, ")\n");
    fflush(fp);
    fclose(fp);
    
    printf("log file '%s' saved.\n", filename);
    
    return YES;
}


