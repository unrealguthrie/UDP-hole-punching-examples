#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512

/* 
 * A small struct to hold a UDP endpoint. We'll use this to hold each client's
 * endpoint.
 */
struct peer {
	unsigned int addr;
	unsigned short port;
	unsigned short alias;
};

int main(int argc, char **argv)
{
	int sock;
	unsigned short port;
	struct sockaddr_in serv;
	struct sockaddr *serv_ptr = (struct sockaddr *)&serv;
	struct sockaddr_in cli;
	struct sockaddr * cli_ptr = (struct sockaddr *)&cli;
	int s_sz = sizeof(struct sockaddr_in);

	struct peer peers[2];
	unsigned int p_sz = sizeof(struct peer);

	int i;
	char buf[BUFLEN];

	if(argc < 2) {
		printf("usage: %s <port>\n", argv[0]);
		return 0;
	}

	port = atoi(argv[1]);	
		
	/* Create the server */
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}

	/* Bind the server to the given port */
	memset(&serv, 0, s_sz);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, serv_ptr, s_sz) < 0) {
		perror("bind()");
		goto err_close_sock;
	}

	printf("Server started on port %d\n", port);

	/* Wait for two clients to connect */
	for (i = 0; i < 2; i++) {
		if(recvfrom(sock, buf, BUFLEN, 0, cli_ptr, &s_sz) < 0) {
			perror("recvfrom()");
			goto err_close_sock;
		}

		peers[i].addr = ntohl(cli.sin_addr.s_addr);
		peers[i].port = ntohs(cli.sin_port);
		peers[i].alias = *(unsigned short *)buf;
		printf("Received packet from %s:%d\n", 
				inet_ntoa(cli.sin_addr),
				ntohs(cli.sin_port));
	}

	/* Swap client information */
	for (i = 0; i < 2; i++) {
		cli.sin_addr.s_addr = htonl(peers[i].addr);
		cli.sin_port = htons(peers[i].port);

		printf("Sending to %s:%d\n", inet_ntoa(cli.sin_addr),
				ntohs(cli.sin_port));

		memcpy(buf, &peers[i ^ 1], p_sz);

		if(sendto(sock, buf, p_sz, 0, cli_ptr, s_sz) < 0) {
			perror("sendto()");
			goto err_close_sock;
		}
	}

err_close_sock:
	close(sock);
	return 0;
}
