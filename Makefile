CC ?= gcc
CFLAGS ?= -O2 -std=c89
LDFLAGS +=
CFDEBUG = -g -pedantic -Wall -Wunused-parameter -Wlong-long\
		  -Wsign-conversion -Wconversion -Wimplicit-function-declaration
SRC = src
MAN = man
EXEC = dfc
VERSION = 3.0.0-devel

SRCS = ${SRC}/csv.c  \
	   ${SRC}/dotfile.c \
	   ${SRC}/dfc.c  \
       ${SRC}/list.c \
       ${SRC}/text.c \
       ${SRC}/util.c
       #${SRC}/html.c
       #${SRC}/tex.c

OBJS= ${SRCS:.c=.o}

.PATH: ${SRC}

PREFIX?=/usr/local
BINDIR=${DESTDIR}${PREFIX}/bin
DATADIR=${DESTDIR}${PREFIX}/share
MANDIR=${DATADIR}/man
LOCALEDIR=${DATADIR}/locale

CFLAGS += -DLOCALEDIR=\"${LOCALEDIR}\" -DPACKAGE=\"${EXEC}\" \
		  -DVERSION=\"${VERSION}\"

LANGUAGES= fr

# needed for portability while remaining simple
SYS:= ${shell uname -s | cut -c 1-7}
include ${SYS}.mk

all: ${EXEC}
	${MAKE} -C po all

.c.o:
	${CC} ${CFLAGS} -o $@ -c $<

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install-main: dfc
	test -d ${BINDIR} || mkdir -p ${BINDIR}
	install -m755 dfc ${BINDIR}/dfc

install-data: ${MAN}/dfc.1 ${MAN}/${LANGUAGES}
	test -d ${MANDIR}/man1 || mkdir -p ${MANDIR}/man1
	install -m644 ${MAN}/dfc.1 ${MANDIR}/man1/dfc.1
	for lang in ${MAN}/${LANGUAGES}; do \
		test -d ${MANDIR}/${LANGUAGES}/man1 || mkdir -p ${MANDIR}/${LANGUAGES}/man1; \
		install -m644 $${lang}/dfc.1 ${MANDIR}/${LANGUAGES}/man1/dfc.1; \
	done

install-po:
	${MAKE} -C po install

install: all install-main install-data install-po

debug: ${EXEC}
debug: CC += ${CFDEBUG}

clean:
	rm -rf ${SRC}/*.o
	${MAKE} -C po clean

mrproper: clean
	rm ${EXEC}

.PHONY: all clean mrproper install install-main install-data install-po
