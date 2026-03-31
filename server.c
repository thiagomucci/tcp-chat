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

struct client
{
	char name[32];
	int logged;
	char ip[16];
} clients[MAXCLI];

void accept_client(int socketfd, struct pollfd fds[], int *nfds)
{

	if(fds[0].revents & POLLIN)
	{
		struct sockaddr_in cliaddr;
		socklen_t len = sizeof(cliaddr);

		int clifd = accept(socketfd, (struct sockaddr *)&cliaddr, &len);
		if(clifd < 0)
		{
			perror("accept");
			return;
		}
		if(*nfds < MAXCLI)
		{
			memset(clients[*nfds].name, 0, sizeof(struct client));
			clients[*nfds].logged = 0;
			fds[*nfds].fd = clifd;
			fds[*nfds].events = POLLIN;
			(*nfds)++;
		}
		else
		{
			close(clifd);
		}
		char *msg = "Welcome! Type your name: ";
		write(clifd, msg, strlen(msg));
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
	if(socketfd < 0)
	{
		perror("socket");
		exit(1);
	}

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

	bzero(fds, sizeof(fds));
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
				memset(buf, 0, MAXBUF);
        		if( (n = read(fds[i].fd, buf, MAXBUF)) <= 0)
				{
					close(fds[i].fd);
    				for(int k = i; k < nfds - 1; k++)
    				{
        				fds[k] = fds[k + 1];
						clients[k] = clients[k + 1];

    				}
					nfds--;
					i--;
				}
			else
			{
				if(clients[i].logged == 0)
				{
					clients[i].logged = 1;
					if (n > 0) 
					{
   						buf[n] = '\0';
						buf[strcspn(buf, "\n\r")] = '\0';
					}
					if (strlen(buf) == 0) 
					{
        				continue; 
    				}
					strncpy(clients[i].name, buf, sizeof(clients[i].name) -1);
					clients[i].name[strcspn(clients[i].name, "\n")] = '\0';
				}
				else
				{
					for(int j = 1; j < nfds; j++)
					{
    					if(j != i)
						{
							char message[MAXBUF + 100];
							memset(message,0, sizeof(message));
							snprintf(message, sizeof(message),"[%s]: %s", clients[i].name, buf);
        					write(fds[j].fd, message, strlen(message));				
						}
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
				clients[k] = clients[k + 1];
    		}

    		nfds--;
    		i--;
		}	
	}
}
}
	return 0;
}

