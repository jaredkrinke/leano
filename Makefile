# TODO: LTO
CC=gcc
CFLAGS=-Wall -Werror -O2
LIBS=-lm -ldl -lpthread

leano: leano.o quickjs/libquickjs.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

leano.o: leano.c lib.h

quickjs/libquickjs.a: quickjs
	make -C quickjs libquickjs.a

lib.h: stringify.sed leano.js
	cat leano.js |sed -f stringify.sed > lib.h

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

