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
#define PORT 9930

// A small struct to hold a UDP endpoint. We'll use this to hold each client's endpoint.
struct peer {
	unsigned int host;
	unsigned int port;
};

// Just a function to kill the program when something goes wrong.
void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in si_me, si_other;
	int s, i, j, slen=sizeof(si_other);
	char buf[BUFLEN];
	struct peer peers[2];
	int n = 0;

	// Create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	// server cannot be behind a NAT.
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr*)(&si_me), sizeof(si_me))==-1)
		diep("bind");

	while(1) {
		if(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)(&si_other), &slen) < 0)
			diep("recvfrom");

		printf("Received packet from %s:%d\n", 
				inet_ntoa(si_other.sin_addr), 
				ntohs(si_other.sin_port));

		if (sendto(s, "hi", 2, 0, (struct sockaddr*)&si_other, slen) < 0)
			diep("sendto");
	}

	close(s);
	return 0;
}
