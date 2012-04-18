PREFIX ?= /usr/local

EXEC = dfc
VERSION = 3.0.0-devel
DATADIR = ${DESTDIR}${PREFIX}/share
LOCALEDIR = ${DATADIR}/locale

SRC = ../src

SRCS = ${SRC}/csv.c \
	   ${SRC}/dfc.c \
	   ${SRC}/dotfile.c \
	   ${SRC}/text.c \
	   ${SRC}/util.c

XG_ARGS = --keyword=_ --keyword=N_ -w 80 --package-name=${EXEC} \
		  --package-version=${VERSION} \
		  --msgid-bugs-address=robin.hahling@gw-computing.net

LANGUAGES= fr

LANGUAGES_PO = ${LANGUAGES:=.po}
LANGUAGES_MO = ${LANGUAGES:=.mo}

.SUFFIXES: .po .mo

all: ${LANGUAGES_MO}

${EXEC}.pot:
	xgettext ${XG_ARGS} --directory=${SRC} ${SRCS} -d ${EXEC} -o ./${EXEC}.pot

update-pot: ${EXEC}.pot

%.pox: %.po
	msgmerge -o $@ --previous $< ${EXEC}.pot

.po.mo:
	msgfmt -v --statistics -c -o $@ $<

install: all
	for po in ${LANGUAGES}; do\
		test -d ${LOCALEDIR}/$${po}/LC_MESSAGES || mkdir -p ${LOCALEDIR}/$${po}/LC_MESSAGES; \
		install -m 0644 $${po}.mo ${LOCALEDIR}/$${po}/LC_MESSAGES/${EXEC}.mo; \
	done

clean-build:
	rm -f *.mo

clean-pox:
	rm -f *.pox

clean: clean-build
	rm -f *~ dfc.pot

.PHONY: all build install clean clean-pox clean-build \
	update-pot
