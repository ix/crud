CC ?= gcc
PREFIX ?= /usr/local
CFLAGS ?= -Wall -pedantic
CFLAGS += `pkg-config --cflags x11 xext` -std=gnu99
LDFLAGS += `pkg-config --libs x11 xext`

all:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS)

debug:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS) -g

install:
	install -D -m755 crud $(DESTDIR)$(PREFIX)/bin/crud

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/crud
