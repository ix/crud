CC ?= gcc
PREFIX ?= /usr/local
CFLAGS = `pkg-config --cflags x11` -std=gnu99 -Wall -Werror -pedantic
LDFLAGS = `pkg-config --libs x11`

all:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS)

debug:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS) -g

install:
	install -m755 crud $(DESTDIR)$(PREFIX)/bin/crud

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/crud
