// UDP hole punching example, server code
// Base UDP code stolen from http://www.abc.se/~m6695/udp.html
// By Oscar Rodriguez
// This code is public domain, but you're a complete lunatic
// if you plan to use this code in any real program.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 512
#define SERV_PORT 9930

struct peer {
	unsigned int addr;
	unsigned short port;
};

int main(void)
{
	struct sockaddr_in serv;
	struct sockaddr_in si_other;
	int s_len = sizeof(struct sockaddr_in);
	int sockfd, j;
	char buf[BUFLEN];
	struct peer peers[2];
	int i = 0;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}

	memset(&serv, 0, s_len);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(SERV_PORT);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&serv, s_len) < 0) {
		perror("bind()");
		return -1;
	}

	for(i = 0; i < 2; i++) {
		if(recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &s_len) < 0) {
			perror("recvfrom()");
			goto err_close_sockfd;
		}

		printf("Received packet from %s:%d\n", 
				inet_ntoa(si_other.sin_addr),
				ntohs(si_other.sin_port));

		peers[i].addr = ntohl(si_other.sin_addr.s_addr);
		peers[i].port = ntohs(si_other.sin_port);
	}

	for (i = 0; i < 2; i++) {
		si_other.sin_addr.s_addr = htonl(peers[i].addr);
		si_other.sin_port = htons(peers[i].port);

		printf("Sending to %s:%d\n", inet_ntoa(si_other.sin_addr), 
				ntohs(si_other.sin_port));

		if (sendto(sockfd, &peers[i ^ 1], sizeof(struct peer), 0, 
					(struct sockaddr *)&si_other, s_len) < 0) {
			perror("sendto()");
			goto err_close_sockfd;
		}
	}	

	close(sockfd);
	return 0;

err_close_sockfd:
	close(sockfd);
	return -1;
}
