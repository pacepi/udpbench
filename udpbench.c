#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void die(char *s)
{
	perror(s);
	exit(1);
}

void usage() {
	printf("Usage :\n");
	printf("\t-h : show help message\n");
	printf("\t-a : ip address\n");
	printf("\t-p : port\n");
	printf("\t-c : clients\n");
	printf("\t-t : bench time\n");
	printf("\t-s : send message string, default \"Hello World\"\n");
	printf("\t-l : send message string length, default length of message string\n");

	exit(0);
}

static int time_expired = 0;

static void sig_alarm_handler(int signal)
{
	time_expired = 1;
}

static void bench_loop(int *res_pipe, int benchtime, char *address, int port, char *buf, int buflen)
{
	struct sockaddr_in sa_server, sa_recv;
	int sa_len = sizeof(sa_server), sa_recvlen = sizeof(sa_server);
	int sockfd = 0;
	struct sigaction sa;
	int succ = 0, fail = 0;
	char *recv_buf = NULL;
	int recv_buflen = 32;
	time_t start = time(NULL);

	recv_buf = (char*)malloc(recv_buflen);
	if (recv_buf == NULL) {
		die("malloc recv_buf fail");
	}

	// setup alarm signal handler
	sa.sa_handler = sig_alarm_handler;
	sa.sa_flags = 0;
	if(sigaction(SIGALRM, &sa, NULL))
		die("sigaction fail");

	alarm(benchtime);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("socket fail\n");
	}

	memset((char *) &sa_server, 0, sizeof(sa_server));
	sa_server.sin_family = AF_INET;
	sa_server.sin_port = htons(port);
	if (inet_aton(address , &sa_server.sin_addr) == 0) {
		die("inet_aton() failed\n");
	}

	while(time_expired == 0) {
		if (sendto(sockfd, buf, buflen, 0, (struct sockaddr *) &sa_server, sa_len) == -1) {
			die("sendto()");
		}

		memset(recv_buf,'\0', recv_buflen);
		if (recvfrom(sockfd, recv_buf, recv_buflen, 0, (struct sockaddr *) &sa_recv, &sa_recvlen) == -1) {
			if (errno == EINTR) {
				//time_t finish = time(NULL);
				//printf("[%d]start : %s, finish : %s\n", getpid(), ctime(&start),ctime(&finish));
				break;
			} else
				die("recvfrom() fail");
		}

		//printf("recvfrom : %s\n", recv_buf);
		// TODO try to check result and set succ/fail
		succ++;
	}

	char result[32] = {0};
	memset(result, 0x00, sizeof(result));
	sprintf(result, "%d %d", succ, fail);
	printf("[%d]result : %s\n", getpid(), result);
	write(res_pipe[1], result, sizeof(result));

	close(sockfd);
}

int main(int argc, char *argv[])
{
	int i = 0;
	char *buf = "Hello World";
	int buflen = 0;
	int port = 0;
	int opt = 0;
	int clients = 1;
	int benchtime = 60;
	char *address = "127.0.0.1";
	int total_succ = 0, total_fail = 0;

	while ((opt = getopt(argc, argv, "hva:p:c:t:")) != -1) {
		switch (opt) {
			case 'a':
				address = optarg;
				break;

			case 'p':
				port = atoll(optarg);
				break;

			case 'c':
				clients = atoll(optarg);
				break;

			case 't':
				benchtime = atoll(optarg);
				break;

			case 's':
				buf = optarg;
				break;

			case 'l':
				buflen = atoll(optarg);
				break;

			case 'h':
			default :
				usage();
		}
	}

	if (buflen == 0) {
		buflen = strlen(buf);
	}

	int res_pipe[2];
	if(pipe(res_pipe)) {
		die("pipe failed.");
	}

	pid_t client = 0;
	for (i = 0; i < clients; i++) {
		client = fork();
		if (client < 0) {
			die("fork failed");
		} 
		// child process
		if (client == 0) {
			bench_loop(res_pipe, benchtime, address, port, buf, buflen);
			exit(0);
		}

	}

	while (clients > 0) {
		int succ = 0, fail = 0;
		char result[32] = {0};

		read(res_pipe[0], result, sizeof(result));
		sscanf(result, "%d %d", &succ, &fail);

		total_succ += succ;
		total_fail += fail;

		clients--;
	}

	printf("Result :\n");
	printf("\tTotal succ : %012d, Avg succ : %012d\n", total_succ, total_succ / benchtime);
	printf("\tTotal fail : %012d, Avg fail : %012d\n", total_fail, total_fail / benchtime);

	return 0;
}
