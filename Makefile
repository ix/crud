CC?=gcc
CFLAGS=-std=gnu99 -Wall -Werror -pedantic
LDFLAGS=-lX11
all:
	$(CC) srect.c -o srect $(LDFLAGS) $(CFLAGS)

debug:
	$(CC) srect.c -o srect $(LDFLAGS) $(CFLAGS) -g
