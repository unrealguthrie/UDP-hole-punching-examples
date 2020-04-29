CC := gcc

all: server client

server: udp_server.c
	$(CC) -o $@ $^

client: udp_client.c
	$(CC) -o $@ $^

clean:
	rm server client
