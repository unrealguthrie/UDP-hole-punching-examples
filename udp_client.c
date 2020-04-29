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
#include <stdint.h>

#define BUFLEN 512
#define NPACK 10

/* 
 * This is our server's IP address and port.
 */
#define SRV_IP "78.46.187.177"
#define SERV_PORT 9930

/*
 * The default-port for peers. 
 */
#define CLI_PORT 14242

/* 
 * A small struct to hold a UDP endpoint. We'll use this to hold each peer's
 * endpoint.
 */
struct peer
{
	unsigned int addr;
	unsigned short port;
};

int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	struct sockaddr_in cli;
	struct sockaddr_in test;
	unsigned int s_len = sizeof(struct sockaddr_in);
	int sockfd;
	int r;
	char buf[BUFLEN];
	struct peer other;
	int i, p;
	uint8_t flg = 0;
	uint8_t res = 0;
	struct timeval read_timeout;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}

	memset(&cli, 0, s_len);
	cli.sin_family = AF_INET;
	cli.sin_port = htons(CLI_PORT - 1);
	cli.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&cli, s_len) < 0) {
		perror("bind()");
		goto err_close_sockfd;
	}

	/* The server's endpoint data */
	memset((char *) &si_other, 0, s_len);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(SERV_PORT);
	if(inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
		perror("inet_aton()");
		goto err_close_sockfd;
	}

	if(sendto(sockfd, "hi", 2, 0, (struct sockaddr *)&si_other, s_len) < 0) {
		perror("sendto()");
		goto err_close_sockfd;
	}

	if(recvfrom(sockfd, &other, sizeof(struct peer), 0, NULL, NULL) < 0) {
		perror("recvfrom()");
		goto err_close_sockfd;
	}

	si_other.sin_addr.s_addr = htonl(other.addr);
	si_other.sin_port = htons(CLI_PORT);

	printf("add peer %s:%d\n",  inet_ntoa(si_other.sin_addr),
			si_other.sin_port);

	close(sockfd);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}

	memset(&cli, 0, s_len);
	cli.sin_family = AF_INET;
	cli.sin_port = htons(CLI_PORT);
	cli.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&cli, s_len) < 0) {
		perror("bind()");
		goto err_close_sockfd;
	}

	if((r = sendto(sockfd, &flg, 1, 0, (struct sockaddr *)&si_other, s_len)) < 0) {
		perror("sendto()");
		goto err_close_sockfd;
	}

	printf("Send %d byte(s) to %s:%d\n", r, inet_ntoa(si_other.sin_addr),
			si_other.sin_port);

	while(1) {
		if((r = recvfrom(sockfd, buf, 10, 0, (struct sockaddr *)&test, &s_len)) > 0) {
			printf("Received %d byte(s) from %s:%d\n", r, 
					inet_ntoa(test.sin_addr),
					ntohs(test.sin_port));
		}
	}

	close(sockfd);
	return 0;

err_close_sockfd:
	close(sockfd);
	return -1;
}
