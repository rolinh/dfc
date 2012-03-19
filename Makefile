CC	?= gcc
CFLAGS = -O3 -std=c89
#CFLAGS += -g3 -std=c89 -pedantic -Wall -Wunused-parameter
#CFLAGS += -Wlong-long -Wsign-conversion -Wconversion -Wimplicit-function-declaration
LDFLAGS +=
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

install-data:
	install -m644 dfc.1 ${MANDIR}/man1

install: all install-main install-data

clean:
	rm -rf *.o

mrproper: clean
	rm ${EXEC}

.PHONY: all clean mrproper install install-main install-data
