CFLAGS  ?= -O3 -DEPOLL
LDFLAGS ?= -ldb
PREFIX  ?= /usr/local
BINDIR  ?= ${PREFIX}/bin

OBJS += dbhttpd

all: httpbuild ${OBJS}

httpbuild:
	mkdir httpbuild
	cp -r httpserver.h/src httpbuild
	cd httpserver.h && cmake -B ../httpbuild
	cd httpbuild && make httpserver.h

install: all
	install ${OBJS} ${DESTDIR}${BINDIR}

uninstall:
	rm -f ${OBJS:%=${DESTDIR}${BINDIR}/%}

clean:
	rm -rf httpbuild
	rm -f  ${OBJS}
