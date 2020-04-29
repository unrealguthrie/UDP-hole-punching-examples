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

int main(int argc, char* argv[])
{
	struct sockaddr_in si_other;
	int sockfd, k;
	unsigned int slen = sizeof(si_other);
	char buf[BUFLEN];
	struct peer other;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("failed to create socket");
		return -1;
	}

	/* The server's endpoint data */
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	if(inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
		perror("failed to convert server-ip");
		goto err_close_sockfd;
	}

	if(sendto(sockfd, "hi", 2, 0, (struct sockaddr*)&si_other, slen) < 0) {
		perror("failed to send to server");
		goto err_close_sockfd;
	}

	if(recvfrom(sockfd, &other, sizeof(struct peer), 0, 
				(struct sockaddr*)&si_other, &slen) < 0) {
		perror("failed to receive from server");
		goto err_close_sockfd;
	}

	printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), 
			ntohs(si_other.sin_port));

	si_other.sin_addr.s_addr = htonl(other.host);
	si_other.sin_port = htons(other.port);
	printf("add peer %s:%d\n", inet_ntoa(si_other.sin_addr), 
			ntohs(si_other.sin_port));

	if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		perror("failed to set non-blocking");
		goto err_close_sockfd;
	}

	while(1) {
		if(sendto(sockfd, "hi", 2, 0, (struct sockaddr *)&si_other, slen) < 0) {
			perror("failed to send to peer");
			goto err_close_sockfd;
		}

		if(recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&si_other, &slen) > 0) {
			printf("Received packet %s from %s:%d\n", buf, 
					inet_ntoa(si_other.sin_addr),
					ntohs(si_other.sin_port));
		}

		usleep(50000);
	}

	close(sockfd);
	return 0;

err_close_sockfd:
	close(sockfd);
	return -1;
}
