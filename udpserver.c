#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN (8*1024)
#define SEQUENCE 10000

enum service_type {
	service_echo,
	service_daytime,
};

void die(char *s)
{
	perror(s);
	exit(1);
}

int parse_service_type(char *type_str)
{
	int service_type = service_echo;

	if (!strcmp(type_str, "echo")) {
		service_type = service_echo;
	} else if (!strcmp(type_str, "daytime")) {
		service_type = service_daytime;
	}

	return service_type;
}

void usage() {
	printf("Usage :\n");
	printf("\t-h : show help message\n");
	printf("\t-p : port\n");
	printf("\t-s : supported service : echo(default), daytime\n");
	printf("\t-v : show sequence info per-%d\n", SEQUENCE);
	printf("\t-l : response message length, default string length(max : %d)\n", BUFLEN);

	exit(0);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in si_server, si_client;
	int sa_len = sizeof(si_server);
	int ca_len = sizeof(si_client);
	char buf[BUFLEN];
	unsigned long sequence = 0;
	int port = 0;
	int opt = 0;
	int service_type = service_echo;
	int buflen = BUFLEN;
	int show_seq = 0;
	int sockfd = 0;

	while ((opt = getopt(argc, argv, "hp:s:l:v")) != -1) {
		switch (opt) {
			case 'p':
				port = atoll(optarg);
				break;

			case 's':
				service_type = parse_service_type(optarg);
				break;

			case 'l':
				buflen = atoll(optarg);
				if (buflen > BUFLEN)
					die("-l option overflow, check -h option");
				break;

			case 'v':
				show_seq = 1;
				break;

			case 'h':
			default :
				usage();
		}
	}

	if (port == 0) {
		switch (service_type) {
			case service_echo :
				port = 7;
				break;

			case service_daytime :
				port = 13;
				break;
		}
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		die("socket");

	memset((char *) &si_server, 0, sizeof(si_server));
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(port);
	si_server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, &si_server, sa_len)==-1)
		die("bind");

	printf("bind port : %d, running ...\n", port);

	while (1) {
		size_t ret = 0;
		ret = recvfrom(sockfd, buf, BUFLEN, 0, &si_client, &ca_len);
		if (ret == -1)
			die("recvfrom()");

		if (service_type == service_echo) {
			ret = sendto(sockfd, buf, ret, 0, &si_client, ca_len);
		} else if (service_type == service_daytime) {
			time_t now = time(NULL);
			memset(buf, 0x00, buflen);
			sprintf(buf, "%s", ctime(&now));

			ret = sendto(sockfd, buf, strlen(buf), 0, &si_client, ca_len);
		}
		if (ret == -1)
			die("sendto()");

		if (show_seq) {
			sequence++;
			if (sequence % 10000 == 0)
				printf("sequence : %lu\n", sequence);
		}
	}

	close(sockfd);
	return 0;
}
