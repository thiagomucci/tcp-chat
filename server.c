#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXBUF 4096
#define MAXCLI 10

void accept_client(int socketfd, struct pollfd fds[], int *nfds)
{
			if(fds[0].revents & POLLIN)
			{
			int clifd = accept(socketfd,NULL,NULL);
			if(clifd < 0)
			{
				perror("accept");
				return;
			}
			fds[*nfds].fd = clifd;
			fds[*nfds].events = POLLIN;
			(*nfds)++;
			}
}

int create_server(int port)
{
	struct sockaddr_in servaddr;

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_port = htons(port);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);


	if(bind(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
	perror("bind");
	exit(1);
	}

	if(listen(socketfd, 10) < 0)
	{
		perror("listen");
		exit(1);
	}
	return socketfd;
}


int
main(int argc, char *argv[])
{	
	if(argc != 2)
	{
		write(STDERR_FILENO,"try: ./prog <port>\n", sizeof("try: ./prog <port>\n") -1);
		exit(1);
	}
	char buf[MAXBUF];
	int n = 0;

	int port = strtol(argv[1], NULL, 10);
	int socketfd = create_server(port);

	struct pollfd fds[MAXCLI];
	int nfds = 1;

	bzero(&fds, sizeof(fds));
	fds[0].fd = socketfd;
	fds[0].events = POLLIN;
	
while(1)
{
	int ready = poll(fds, nfds, -1);
	if(ready > 0)
	{
	accept_client(socketfd, fds, &nfds);
	for(int i = 1; i < nfds; i++)
	{
    	if(fds[i].revents & POLLIN)
		{
        	if( (n =read(fds[i].fd, buf, MAXBUF)) <= 0)
			{
				close(fds[i].fd);
				nfds--;
				i--;
			}

		else
		{
			for(int j = 1; j < nfds; j++)
			{
    			if(j != i)
				{
        			write(fds[j].fd, buf, n);
									
				}
			}
		}
   		}
    	else if(fds[i].revents & POLLHUP)
		{
    		close(fds[i].fd);
    		for(int k = i; k < nfds - 1; k++)
    		{
        		fds[k] = fds[k + 1];
    		}

    	nfds--;
    	i--;
		}	
	}
}
	return 0;
}
