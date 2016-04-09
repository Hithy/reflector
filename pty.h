#pragma once
#include <termios.h>

#define TTY_BUF_SIZE 1024

#define TTY_RAW_MODE 0
#define TTY_NORMAL_MODE 1

extern pid_t pty_fork(int *ptrfdm, int mode);
extern int start_pty(char **env, int fd, int mode);