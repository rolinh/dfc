CC	?= gcc
CFLAGS = -O2 -std=c89
LDFLAGS +=
CFDEBUG = -g3 -std=c89 -pedantic -Wall -Wunused-parameter -Wlong-long\
		  -Wsign-conversion -Wconversion -Wimplicit-function-declaration
EXEC = dfc
OBJS = dfc.o

PREFIX=/usr/local
BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/man

all: ${EXEC}

dfc.o: dfc.c

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install-main: dfc
	install -m755 dfc ${BINDIR}

install-data: dfc.1
	install -Dm644 dfc.1 ${MANDIR}/man1/dfc.1

install: all install-main install-data

debug: ${OBJS}
	${CC} ${CFDEBUG} ${LDFLAGS} -o ${EXEC} ${OBJS}

clean:
	rm -rf *.o

mrproper: clean
	rm ${EXEC}

.PHONY: all clean mrproper install install-main install-data
