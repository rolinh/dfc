CC	?= gcc
CFLAGS ?= -O2 -std=c89
LDFLAGS +=
CFDEBUG = -g3 -pedantic -Wall -Wunused-parameter -Wlong-long\
		  -Wsign-conversion -Wconversion -Wimplicit-function-declaration
SRC = src
MAN = man
EXEC = dfc
OBJS = ${SRC}/dfc.o ${SRC}/list.o

PREFIX=/usr/local
BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/man

all: ${EXEC}

dfc.o: ${SRC}/dfc.c

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install-main: dfc
	test -d ${DESTDIR}${BINDIR} || mkdir -p ${DESTDIR}${BINDIR}
	install -m755 dfc ${DESTDIR}${BINDIR}/dfc

install-data: ${MAN}/dfc.1
	test -d ${DESTDIR}${MANDIR}/man1 || mkdir -p ${DESTDIR}${MANDIR}/man1
	install -m644 ${MAN}/dfc.1 ${DESTDIR}${MANDIR}/man1/dfc.1

install: all install-main install-data

debug: ${EXEC}
debug: CC += ${CFDEBUG}

clean:
	rm -rf ${SRC}/*.o

mrproper: clean
	rm ${EXEC}

.PHONY: all clean mrproper install install-main install-data
