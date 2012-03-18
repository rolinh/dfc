CC	?= gcc
#CFLAGS	+= -g3 -std=c89 -pedantic -Wall -Wunused-parameter
CFLAGS += -g3 -std=c89 -pedantic -Wall -Wextra -Wunused-parameter -Wlong-long -Wsign-conversion -Wconversion -Wimplicit-function-declaration
LDFLAGS +=
EXEC = dfc
OBJS = dfc.o

PREFIX=/usr/local

all: ${EXEC}

dfc.o: dfc.c

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

install:
	install -m 0755 dfc ${PREFIX}/bin

clean:
	rm -rf *.o

mrproper: clean
	rm ${EXEC}

.PHONY: clean mrproper install
