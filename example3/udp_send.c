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

int main(int argc, char **argv)
{
	struct sockaddr_in other;
	int s_len = sizeof(struct sockaddr_in);
	int sockfd;
	int j, i;

	if(argc < 3) {
		printf("usage: %s <addr> <port>\n", argv[0]);
		return -1;
	}

	for(i = 0; i < 10; i++) {
		if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("socket()");
			break;
		}

		memset(&other, 0, s_len);
		other.sin_family = AF_INET;
		other.sin_port = htons(atoi(argv[2]));
		inet_pton(AF_INET, argv[1], (struct sockaddr *)&other.sin_addr);

		if(sendto(sockfd, "hi", 2, 0, (struct sockaddr *)&other, s_len) < 0) {
			perror("sendto()");
			break;
		}

		close(sockfd);
	}
	return 0;
}

