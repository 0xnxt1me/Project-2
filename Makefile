CFLAGS=-Wall -std=c11 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g -fprofile-arcs -ftest-coverage 
CC=gcc

CFLAGS +=  -g -fsanitize=address -fprofile-arcs -ftest-coverage 

LDFLAGS=-fsanitize=address  -fprofile-arcs -ftest-coverage 

PWM_OBJS=pwm.o commands.o core.o utils.o md5.o validation.o 

all: pwm

pwm: $(PWM_OBJS) 
	$(CC) $(LDFLAGS) $(PWM_OBJS) -lbsd -o pwm

clean:
	rm -f *.o pwm *.plist *.gcda *.gcno *.gcov leak-* crash-*
