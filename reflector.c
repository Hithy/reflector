#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pty.h"
#include "misc.h"
#include "net.h"


int main(int argc, char *argv[])
{

	int spid;
	char logbuf[1024] = { 0 };

	struct sockaddr_in client_addr;
	int sfd;
	int cfd;
	int client_addrlen = sizeof(client_addr);


	if (argc < 3)
	{
		printf("You should enter the program\n");
		return -1;
	}

	signal(SIGCHLD, SIG_IGN);
	if (access(argv[2], F_OK))
	{
		printf("File not exist\n");
		exit(0);
	}
	
	log_init("connected.log");
	net_init(&sfd, atoi(argv[1]), NET_INET);

	while (1)
	{
		cfd = accept(sfd, (struct sockaddr *) &client_addr, &client_addrlen);

		if (access(argv[2], F_OK))
		{
			printf("File not exist\n");
			close(cfd);
			exit(0);
		}
		if (cfd != -1)
		{
			//set_noecho(cfd);
			spid = fork();
			if (spid == 0)
			{
				start_pty(&argv[2], cfd, TTY_RAW_MODE);
				exit(0);
			}
			else
			{
				memset(logbuf, 0, 1024);
				sprintf(logbuf, "[%d] Connected from %s -- %s", spid, inet_ntoa(client_addr.sin_addr), get_actime());
				write_log(logbuf);
			}
			close(cfd);
		}
	}
}