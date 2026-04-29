CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: server client

server: orion.c arena.h
	$(CC) $(CFLAGS) orion.c -o orion

client: eternal.c arena.h
	$(CC) $(CFLAGS) eternal.c -o eternal

clean:
	rm -f orion eternal

clear_ipc:
	ipcrm -M 0x00009012 2>/dev/null || true
	ipcrm -Q 0x00001234 2>/dev/null || true
