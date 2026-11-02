CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
SRC = src/mcformat.c src/mclib.c

all: mcinfo selftest

mcinfo: examples/mcinfo.c src/mcformat.c
	$(CC) $(CFLAGS) -Isrc -o $@ examples/mcinfo.c src/mcformat.c

selftest: tests/selftest.c $(SRC)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/selftest.c $(SRC)

test: selftest
	./selftest

clean:
	rm -f mcinfo selftest
