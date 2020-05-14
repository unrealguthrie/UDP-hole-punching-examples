#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SRV_IP "78.46.187.177"
#define SRV_PORT 9930

#define SOCK_NUM  2
#define BUF_LEN 512

struct peer {
	unsigned int addr;
	unsigned short port;
	unsigned short alias;
};

struct sock {
	int fd;
	unsigned short port;
	uint8_t used;
};

int main(int argc, char **argv)
{
	struct sock socks[SOCK_NUM];
	struct sockaddr_in cli;
	struct sockaddr *cli_ptr = (struct sockaddr *)&cli;
	struct sockaddr_in serv;
	struct sockaddr *serv_ptr = (struct sockaddr *)&serv;
	struct sockaddr_in peer;
	struct sockaddr *peer_ptr = (struct sockaddr *)&peer;
	unsigned int s_sz = sizeof(struct sockaddr_in);

	struct peer other;
	unsigned int p_sz = sizeof(struct peer);

	struct timeval tv;
	fd_set rfds;

	int port;
	char buf[BUF_LEN];
	int p;
	int i;

	if(argc < 2) {
		printf("usage: %s <port>\n", argv[0]);
		return -1;
	}

	port = atoi(argv[1]);

	tv.tv_sec = 0;
	tv.tv_usec = 500000;

	/*
	 * Initialize all sockets and set them on different ports.
	 */

	for(i = 0; i < SOCK_NUM; i++) {
		socks[i].fd = -1;
	}

	for(i = 0; i < SOCK_NUM; i++) {	
		if((socks[i].fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("socket()");
			return -1;
		}

		socks[i].port = port + i;
		socks[i].used = 0;

		/* Bind the socket to the designated port */
		memset(&cli, 0, s_sz);
		cli.sin_family = AF_INET;
		cli.sin_port = htons(socks[i].port);
		cli.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(socks[i].fd, cli_ptr, s_sz) < 0) {
			perror("bind()");
			return -1;
		}

		/* If this socket is not the server-socket, then set timeout */
		if(i != 0) {
			if(setsockopt(socks[1].fd, SOL_SOCKET, SO_RCVTIMEO, 
						&tv, sizeof(tv)) < 0) {
				perror("setsockopt()");
				return -1;
			}
		}
	}

	memset(&serv, 0, s_sz);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(SRV_PORT);
	if(inet_aton(SRV_IP, &serv.sin_addr) < 0) {
		perror("aton()");
		goto err_close_sock;
	}

	printf("Send to server\n");
	if(sendto(socks[0].fd, &socks[1].port, 2, 0, serv_ptr, s_sz) < 0) {
		perror("sendto()");
		goto err_close_sock;
	}

	printf("Receive from server\n");
	if(recvfrom(socks[0].fd, &other, p_sz, 0, NULL, NULL) < 0) {
		perror("recv()");
		goto err_close_sock;
	}

	memset(&peer, 0, s_sz);
	peer.sin_family = AF_INET;
	peer.sin_port = htons(other.alias);
	peer.sin_addr.s_addr = htonl(other.addr);

	for(p = 0; p < 10; p++) {
		printf("Send packet to %s:%d\n", inet_ntoa(peer.sin_addr),
				ntohs(peer.sin_port));

		for(i = 0; i < 5; i++) {
			if(sendto(socks[1].fd, "hi\0", 3, 0, peer_ptr, s_sz) < 0) {
				perror("sendto()");
				goto err_close_sock;
			}
		}

		if(recvfrom(socks[1].fd, buf, BUF_LEN, 0, NULL, NULL) > 0) {
			printf("Received packet '%s'\n", buf);
		}

	}

err_close_sock:
	for(i = 0; i < SOCK_NUM; i++) {
		if(socks[i].fd > 0)
			close(socks[i].fd);
	}
	return 0;
}
