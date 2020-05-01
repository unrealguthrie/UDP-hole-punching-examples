/*
 * UDP hole punching example, client code
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
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUFLEN 512
#define NPACK 10

/*
 * This is our server's IPv4-address and port.
 */
#define SRV_IP "78.46.187.177"
#define SRV_PORT 9930

#define SOCK_PORT 9931
#define SOCK_NUM 5

/*
 * A small struct to hold a UDP endpoint. We'll use this to hold each 
 * peer's endpoint.
 */
struct peer {
	unsigned int addr;
	unsigned short port;
};

struct sock {
	int fd;
	unsigned short port;
	uint8_t used;
};

int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	struct sockaddr_in si_recv;
	struct sockaddr_in cli;
	unsigned int s_sz = sizeof(struct sockaddr_in);
	struct sock socks[SOCK_NUM];
	int p, i;
	char buf[BUFLEN];
	struct peer other;
	unsigned int p_sz = sizeof(struct peer);
	struct sockaddr *si_ptr = (struct sockaddr *)&si_other;
	struct timeval tv;
	fd_set rfds;

	tv.tv_sec = 0;
	tv.tv_usec = 20000;

	for(i = 0; i < SOCK_NUM; i++) {
		socks[i].fd = -1;
	}

	FD_ZERO(&rfds);

	/*
	 * Initialize all sockets and set them on different ports.
	 */
	for(i = 0; i < SOCK_NUM; i++) {	
		if((socks[i].fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("socket()");
			return -1;
		}

		FD_SET(socks[i].fd, &rfds);
		socks[i].port = SOCK_PORT + i;
		socks[i].used = 0;

		/*
		 * Bind the socket to the designated port-number.
		 */
		memset(&cli, 0, s_sz);
		cli.sin_family = AF_INET;
		cli.sin_port = htons(socks[i].port);
		cli.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(socks[i].fd, (struct sockaddr *)&cli, s_sz) < 0) {
			perror("bind()");
			goto err_close_sock;
		}
	}

	memset(&si_other, 0, s_sz);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(SRV_PORT);
	if(inet_aton(SRV_IP, &si_other.sin_addr) < 0) {
		perror("aton()");
		goto err_close_sock;
	}

	printf("Send to server\n");
	if(sendto(socks[0].fd, &socks[1].port, sizeof(unsigned short), 0, si_ptr, s_sz) < 0) {
		perror("sendto()");
		goto err_close_sock;
	}

	printf("Receive from server\n");
	if(recvfrom(socks[0].fd, &other, p_sz, 0, si_ptr, &s_sz) < 0) {
		perror("recv()");
		goto err_close_sock;
	}

	printf("Init client\n");
	si_other.sin_addr.s_addr = htonl(other.addr);
	si_other.sin_port = htons(other.port);
	printf("add peer %s:%d\n", inet_ntoa(si_other.sin_addr),
		ntohs(si_other.sin_port));

	for(p = 0; p < 10; p++) {
		printf("Send packet to %s:%d\n", inet_ntoa(si_other.sin_addr),
			ntohs(si_other.sin_port));
		if(sendto(socks[1].fd, "hi\0", 3, 0, si_ptr, s_sz) < 0) {
			perror("sendto()");
			goto err_close_sock;
		}

		if(recvfrom(socks[1].fd, buf, BUFLEN, 0, (struct sockaddr *)&si_recv, &s_sz) > 0) {
			printf("Received packet %s from %s:%d\n", buf, 
			inet_ntoa(si_recv.sin_addr),
			ntohs(si_recv.sin_port));
		}

	}

err_close_sock:
	for(i = 0; i < SOCK_NUM; i++) {
		if(socks[i].fd > 0)
			close(socks[i].fd);
	}
	return 0;
}
