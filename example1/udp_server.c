/*
 * UDP hole punching example, server code
 * Base UDP code stolen from http://www.abc.se/~m6695/udp.html
 * By Oscar Rodriguez
 * This code is public domain, but you're a complete lunatic
 * if you plan to use this code in any real program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512
#define PORT 9930

/* 
 * A small struct to hold a UDP endpoint. We'll use this to hold each client's
 * endpoint.
 */
struct peer {
	unsigned int addr;
	unsigned short port;
};

int main(int argc, char **argv)
{		
	struct sockaddr_in si_me;
	struct sockaddr *s_me = (struct sockaddr  *)&si_me;
	struct sockaddr_in si_other;
	struct sockaddr *s_other = (struct sockaddr *)&si_other;
	unsigned int s_sz = sizeof(si_other);
	int sock, i;
	char buf[BUFLEN];
	struct peer peers[2];
	unsigned int p_sz = sizeof(struct peer);

	if(argc || argv) {/* Prevent warning for not using parameters */}

	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}

	memset(&si_me, 0, s_sz);
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, s_me, s_sz) < 0) {
		perror("bind()");
		goto err_close_sock;
	}

	for (i = 0; i < 2; i++) {
		if(recvfrom(sock, buf, BUFLEN, 0, s_other, &s_sz) < 0) {
			perror("recvfrom()");
			goto err_close_sock;
		}
		peers[i].addr = ntohl(si_other.sin_addr.s_addr);
		peers[i].port = ntohs(si_other.sin_port);
		printf("Received packet from %s:%d\n", 
				inet_ntoa(si_other.sin_addr),
				ntohs(si_other.sin_port));
	}

	for (i = 0; i < 2; i++) {
		si_other.sin_addr.s_addr = htonl(peers[i].addr);
		si_other.sin_port = htons(peers[i].port);

		printf("Sending to %s:%d\n", inet_ntoa(si_other.sin_addr),
				ntohs(si_other.sin_port));

		memcpy(buf, &peers[i ^ 1], p_sz);
		*(buf + p_sz) = i;

		if(sendto(sock, buf, p_sz + 1, 0, s_other, s_sz) < 0) {
			perror("sendto()");
			goto err_close_sock;
		}
	}

err_close_sock:
	close(sock);
	return 0;
}
