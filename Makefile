#
# $Id$
#
OUT = othello.exe
OBJS = othello.o output.o command.o util.o put.o think.o
CFLAGS = -g -DDEBUG
#CFLAGS = -O
$(OUT): $(OBJS)


othello.o: othello.c othello.h
output.o: output.c othello.h

.c.o:
	gcc -c $(CFLAGS) $<

$(OUT):
	gcc -o $(OUT) $(OBJS)

clean:
	rm -f othello.exe *.o *~ *stackdump

