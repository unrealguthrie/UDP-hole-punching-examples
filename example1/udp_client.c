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

#define BUFLEN 512
#define NPACK 10

/*
 * This is our server's IPv4-address and port.
 */
#define SRV_IP "78.46.187.177"
#define SRV_PORT 9930

/*
 * A small struct to hold a UDP endpoint. We'll use this to hold each 
 * peer's endpoint.
 */
struct peer
{
	unsigned int addr;
	unsigned short port;
};

int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	struct sockaddr_in si_recv;
	unsigned int s_sz = sizeof(struct sockaddr_in);
	int sock;
	int p, i;
	char buf[BUFLEN];
	struct peer other;
	unsigned int p_sz = sizeof(struct peer);
	struct sockaddr *si_ptr = (struct sockaddr *)&si_other;
	struct timeval tv;

	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket()");
		return -1;
	}
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	memset(&si_other, 0, s_sz);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(SRV_PORT);
	if(inet_aton(SRV_IP, &si_other.sin_addr) < 0) {
		perror("aton()");
		goto err_close_sock;
	}

	if(sendto(sock, "hi", 2, 0, si_ptr, s_sz) < 0) {
		perror("sendto()");
		goto err_close_sock;
	}

	if(recvfrom(sock, buf, BUFLEN, 0, si_ptr, &s_sz) < 0) {
		perror("recv()");
		goto err_close_sock;
	}

	memcpy(&other, buf, p_sz);
	si_other.sin_addr.s_addr = htonl(other.addr);
	si_other.sin_port = htons(other.port);
	printf("add peer %s:%d\n", inet_ntoa(si_other.sin_addr),
		ntohs(si_other.sin_port));

	for(p = 0; p < 10; p++) {
		for(i = 0; i < 10; i++) {
			si_other.sin_port = htons(other.port + i);

			if(sendto(sock, "hi", 2, 0, si_ptr, s_sz) < 0) {
				perror("sendto()");
				goto err_close_sock;
			}
			usleep(10000);
		}
		
		if(recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&si_recv, &s_sz) > 0) {
			printf("Received packet %s from %s:%d\n", buf, 
				inet_ntoa(si_recv.sin_addr),
				ntohs(si_recv.sin_port));
		}
	}

err_close_sock:
	close(sock);
	return 0;
}
