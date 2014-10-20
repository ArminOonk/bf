#!/usr/bin/env make
CC= cc
CWARN= -Wall -pedantic
CSTD= -std=c99
OPT= -O2
ARCH=
LIBS= 
CFLAGS= ${CWARN} ${CSTD} ${ARCH} ${OPT} ${LIBS}

ENTRY= bf

all: ${ENTRY}
	@${TRUE}

${ENTRY}: ${ENTRY}.c
	${CC} $< -o $@ ${CFLAGS}
	
clean:
	rm -f *.o ${ENTRY}
	echo Clean done