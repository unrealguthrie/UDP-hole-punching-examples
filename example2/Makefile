CC        := gcc
CFLAGS    := -Wall -ansi -std=c89 -pedantic

all: server client

server: udp_server.c
	$(CC) -o $@ $^ $(CFLAGS)

client: udp_client.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm server client
