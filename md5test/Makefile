
CFLAGS=-Wall -std=c11 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g 
CC=gcc

CFLAGS +=  -g -fsanitize=address

LDFLAGS=-fsanitize=address

md5test: md5test.c
	$(CC) $(CFLAGS) $(LDFLAGS) ../md5.c md5test.c -o md5test

clean:
	rm -f md5test 
