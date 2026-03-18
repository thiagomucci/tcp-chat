#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXBUF 4096

int
main(int argc, char *argv[])
{
	if(argc != 3)
	{
		write(STDERR_FILENO,"try: ./prog <port>\n", sizeof("try: ./prog <port>\n") -1);
		exit(1);
	}
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	char buf[MAXBUF];
	ssize_t n;

	int port = strtol(argv[1], NULL, 10);
	servaddr.sin_port = htons(port);

	inet_pton(AF_INET, argv[2], &servaddr.sin_addr);
	servaddr.sin_family = AF_INET;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct pollfd fds[2];
	fds[0].fd = STDIN_FILENO; fds[0].events = POLLIN;
	fds[1].fd = sockfd; fds[1].events = POLLIN;

	int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(ret < 0)
	{
		perror("connect");
		exit(1);
	}

	while(1)
	{
		poll(fds, 2, -1);

		if(fds[0].revents & POLLIN)
		{
			n = read(STDIN_FILENO, buf, MAXBUF);
			if(n < 0)
			{
				perror("read");
				exit(1);
			}

			if(n == 0)
			{
				close(sockfd);
				exit(0);
			}
			write(sockfd, buf, n);
		}

		if(fds[1].revents & POLLIN)
		{
			n = read(sockfd, buf, MAXBUF);
			if(n < 0)
			{
				perror("read");
				exit(1);
			}
			if(n == 0)
			{
				close(sockfd);
				exit(0);
			}
			write(STDOUT_FILENO, buf, n);
		}

	}
}
