CC	?= gcc
CFLAGS ?= -O2 -std=c89
LDFLAGS +=
CFDEBUG = -g3 -pedantic -Wall -Wunused-parameter -Wlong-long\
		  -Wsign-conversion -Wconversion -Wimplicit-function-declaration
SRC = src
MAN = man
EXEC = dfc
OBJS = ${SRC}/dfc.o

PREFIX=/usr/local
BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/man

all: ${EXEC}

dfc.o: ${SRC}/dfc.c

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install-main: dfc
	install -Dm755 dfc ${DESTDIR}${BINDIR}/dfc

install-data: ${MAN}/dfc.1
	install -Dm644 ${MAN}/dfc.1 ${DESTDIR}${MANDIR}/man1/dfc.1

install: all install-main install-data

debug: ${EXEC}
debug: CC += ${CFDEBUG}

clean:
	rm -rf ${SRC}/*.o

mrproper: clean
	rm ${EXEC}

.PHONY: all clean mrproper install install-main install-data
