/*
 * UDP hole punching example, client code
 * Base UDP code stolen from http://www.abc.se/~m6695/udp.html
 * By Oscar Rodriguez
 * This code is public domain, but you're a complete lunatic
 * if you plan to use this code in any real program.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9930

#define CLI_PORT 24242

/* 
 * This is our server's IP address. In case you're wondering, this one is an 
 * RFC 5737 address.
 */
#define SRV_IP "78.46.187.177"

/* 
 * A small struct to hold a UDP endpoint. We'll use this to hold each peer's
 * endpoint.
 */
struct peer
{
	unsigned int host;
	unsigned int port;
};

int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	struct sockaddr_in cli;
	int sockfd, k;
	int r;
	unsigned int slen = sizeof(si_other);
	char buf[BUFLEN];
	struct peer other;
	int i, p;

	int wait[3] = {3000, 3000000, 10000000};

	for(p = 0; p < 3; p++) {
		printf("Phase %d:\n", p);
		for(i = 0; i < 5; i++) {
			printf("Test #%d\n", i);
			if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
				perror("socket()");
				return -1;
			}

			// server cannot be behind a NAT.
			memset(&cli, 0, sizeof(struct sockaddr_in));
			cli.sin_family = AF_INET;
			cli.sin_port = htons(CLI_PORT);
			cli.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(sockfd, (struct sockaddr *)&cli, sizeof(struct sockaddr_in)) < 0) {
				perror("bind()");
				goto err_close_sockfd;
			}

			/* The server's endpoint data */
			memset((char *) &si_other, 0, sizeof(si_other));
			si_other.sin_family = AF_INET;
			si_other.sin_port = htons(PORT);
			if(inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
				perror("inet_aton()");
				goto err_close_sockfd;
			}

			if(sendto(sockfd, "hi", 2, 0, (struct sockaddr*)&si_other, slen) < 0) {
				perror("sendto()");
				goto err_close_sockfd;
			}

			if(recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL) < 0) {
				perror("recvfrom()");
				goto err_close_sockfd;
			}

			printf("Recv(%ld): %s\n", strlen(buf), buf);

			close(sockfd);
			usleep(wait[p]);
		}
	}

	return 0;

err_close_sockfd:
	close(sockfd);
	return -1;
}
