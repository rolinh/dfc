CC	?= gcc
CFLAGS	+= -g3 -std=c89 -pedantic -Wall -Wunused-parameter
#CFLAGS	+= -g3 -std=c89 -pedantic -Wall -Wextra -Wunused-parameter -Wlong-long -Wsign-conversion -Wconversion -Wimplicit-function-declaration
LDFLAGS +=
EXEC	= dfc
OBJS	= dfc.o

all: ${EXEC}

dfc.o: dfc.c

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm ${EXEC}
