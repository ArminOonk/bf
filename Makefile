#!/usr/bin/env make
CC= cc
CWARN= -Wall -pedantic
CSTD= -std=c99
OPT= -O2
ARCH=
LIBS= 
CFLAGS= ${CWARN} ${CSTD} ${ARCH} ${OPT} ${LIBS}

ENTRY= bf
program_C_SRCS := $(wildcard *.c)
program_C_OBJS := ${program_C_SRCS:.c=.o}

all: ${ENTRY}
	@${TRUE}

${ENTRY}: $(program_C_OBJS)
	$(LINK.c) $(program_C_OBJS) -o ${ENTRY}
	
clean:
	rm -f *.o ${ENTRY}
	echo Clean done