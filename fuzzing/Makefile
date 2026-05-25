CFLAGS=-Wall -std=c11 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g
CC=gcc
CFLAGS +=  -g -fsanitize=address
LDFLAGS=-fsanitize=address 

PROGRAMS=blab radamsa

all: $(PROGRAMS) 

blab: blab.o
	$(CC) $(LDFLAGS) blab.o  -o blab

radamsa: radamsa.o
	$(CC) $(LDFLAGS) radamsa.o  -o radamsa


clean:
	rm -f *.o $(PROGRAMS) *.plist *.gcda *.gcno *.gcov leak-* crash-*
