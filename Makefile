CC?=gcc
CFLAGS=-std=gnu99 -Wall -Werror -pedantic
LDFLAGS=-lX11
all:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS)

debug:
	$(CC) crud.c -o crud $(LDFLAGS) $(CFLAGS) -g
