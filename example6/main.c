#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 4242
#define PCK_NUM 10

int main(int argc, char **argv)
{
	int i;

	int sockfd;
	struct sockaddr_in cli;
	struct sockaddr *cli_ptr = (struct sockaddr *)&cli;
	struct sockaddr_in peer;
	struct sockaddr *peer_ptr = (struct sockaddr *)&peer;
	int s_sz = sizeof(struct sockaddr_in);

	char *msg = "Hello World!\n";
	int msg_len = strlen(msg);
	char buf[100];
	int r;
	struct timeval tv;

	if(argc < 2) {
		printf("usage: %s <addr>\n", argv[0]);
		return 0;
	}

	/* Create the socket */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return 0;
	}

	/* Set timeout for receive */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

	/* Bind the socket to the predefined port */
	memset(&cli, 0, s_sz);
	cli.sin_family = AF_INET;
	cli.sin_port = htons(PORT);
	cli.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, cli_ptr, s_sz) < 0) {
		perror("bind()");
		goto err_close_sockfd;
	}

	/* Configure the data for the peer */
	memset(&peer, 0, s_sz);
	peer.sin_family = AF_INET;
	peer.sin_port = htons(PORT);
	if(inet_pton(AF_INET, argv[1], &peer.sin_addr) < 1) {
		perror("inet_pton()");
		goto err_close_sockfd;
	}

	while(1) {
		for(i = 0; i < PCK_NUM; i++) {
			if(sendto(sockfd, msg, msg_len, 0, peer_ptr, s_sz) < 0) {
				perror("sendto()");
				goto err_close_sockfd;
			}
		}

		if((r = recvfrom(sockfd, buf, 100, 0, NULL, NULL)) > 0) {
			buf[r] = 0;
			printf("Recv %d: %s\n", r, buf);
		}
	}

err_close_sockfd:
	close(sockfd);
	return 0;
}

