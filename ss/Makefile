# Generic compilation settings
CC=gcc
CFLAGS=-g -Wall -std=c11 \
       -D_XOPEN_SOURCE=500 -D_GNU_SOURCE \
       -fno-asynchronous-unwind-tables

LDFLAGS=-lbsd -fno-asynchronous-unwind-tables

# Executable code on the stack?
NOXSTACK=0

ifeq ($(NOXSTACK),0)
  LDFLAGS += -z execstack
endif

# Canaries 
CANARIES=0

ifeq ($(CANARIES),0)
  CFLAGS += -fno-stack-protector
else
  CFLAGS += -fstack-protector-all 
endif

# Position-independent executable (PIE) setting
PIE=0

ifeq ($(PIE),0)
  LDFLAGS += -no-pie
else
  CFLAGS += -fPIE
  LDFLAGS += -pie 
endif

PWM_SRCS=../pwm.c ../commands.c ../core.c ../utils.c ../md5.c ../validation.c dump_stack.c

PROGRAMS=pwm sc_exec hello

all: $(PROGRAMS) 

pwm: $(PWM_SRCS) 
	$(CC) $(PWM_SRCS) $(CFLAGS) $(LDFLAGS) -o pwm

hello: hello.c dump_stack.c
	$(CC) hello.c dump_stack.c $(CFLAGS) $(LDFLAGS) -o hello

clean:
	rm -f *.o $(PROGRAMS) 
