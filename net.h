#pragma once

#define NET_LOCAL	0
#define NET_INET	1

extern int net_init(int *sock, int port, int flag);