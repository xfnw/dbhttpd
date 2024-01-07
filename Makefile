CFLAGS ?= -O3 -DEPOLL
PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin

OBJS += dbhttpd

all: httpserver.h ${OBJS}

.PHONY: httpserver.h
httpserver.h: httpserver.h/src/httpserver.h

httpserver.h/src/httpserver.h:
	cd httpserver.h && cmake .
	cd httpserver.h && make httpserver.h

install: all
	install ${OBJS} ${DESTDIR}${BINDIR}

uninstall:
	rm -f ${OBJS:%=${DESTDIR}${BINDIR}/%}

clean:
	rm -f ${OBJS}

