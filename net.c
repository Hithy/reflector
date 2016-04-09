#include "net.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int net_init(int *sock, int port, int flag)
{
	int sfd;
	struct sockaddr_in srv_addr;

	if (sock == NULL)
		return -1;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);

	if (flag == NET_INET)
	{
		srv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	}
	else
	{
		srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}
	

	if (bind(sfd, (const struct sockaddr *) &srv_addr, sizeof srv_addr) ==	0)
	{
		printf("Binding socket is OK\n");
	}
	else 
	{
		fprintf(stderr, "Fail to bind the socket: %s\n", strerror(errno));
		return -1;
	}

	if (listen(sfd, 20) == 0) 
	{
		printf("[%d] Start listening\n", getpid());
	}
	else 
	{
		fprintf(stderr, "Fail to start listening: %s\n", strerror(errno));
		return -1;
	}

	*sock = sfd;

	return 0;
}
