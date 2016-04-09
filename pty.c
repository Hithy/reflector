
#include "pty.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>



int pipe_child;

//Set tty to raw mode
static void set_raw_tty(int fd)
{
	struct termios stermios;

	if (tcgetattr(fd, &stermios) < 0)
		perror("tcgetattr error");


	stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	stermios.c_lflag &= ~(ICANON);
	stermios.c_iflag &= ~(ICRNL | INLCR);
	stermios.c_oflag &= ~(OCRNL | ONLCR);

	stermios.c_cc[VMIN] = 1;
	stermios.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &stermios) <0)
		perror("tcsetattr error");
}

//Create a pty, return the master pty device.
int ptym_open(char *pts_name, int pts_namesz)
{
	int		fdm, err;

	printf("[%d] Init pty device...", getpid());
	if ((fdm = posix_openpt(O_RDWR)) < 0)
		return(-1);
	if (grantpt(fdm) < 0)		/* grant access to slave */
		goto errout;
	if (unlockpt(fdm) < 0)		/* clear slave's lock flag */
		goto errout;
	if (ptsname_r(fdm, pts_name, pts_namesz) != 0)	/* get slave's name */
		goto errout;
	pts_name[pts_namesz - 1] = '\0';
	printf("%s ok\n", pts_name);
	return(fdm);			/* return fd of master */

errout:
	err = errno;
	close(fdm);
	errno = err;
	return(-1);
}

int ptys_open(char *pts_name)
{
	int fds;

	if ((fds = open(pts_name, O_RDWR)) < 0)
		return(-1);

	return(fds);
}

//fork a pty. The parent return the master device and the child's IO was redirect to the pty.
pid_t pty_fork(int *ptrfdm, int mode)
{
	int		fdm, fds;
	pid_t	pid;
	char	pts_name[20];

	if ((fdm = ptym_open(pts_name, sizeof(pts_name))) < 0)
	{
		printf("ERROR: can't open master pty: %s, error %d", pts_name, fdm);
		return (-1);
	}

	if (mode == TTY_RAW_MODE)
	{
		set_raw_tty(fdm);
	}
	

	if ((pid = fork()) < 0) {
		return(-1);
	}

	else if (pid == 0) {		/* child */
		printf("[%d] pty fork child\n", getpid());

		close(fdm);

		if (setsid() < 0)
			printf("ERROR: setsid error");

		if ((fds = ptys_open(pts_name)) < 0)
			printf("ERROR: can't open slave pty");

		if (mode == TTY_RAW_MODE)
		{
			set_raw_tty(fds);
		}
		
		if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
			printf("ERROR: dup2 error to stdin");
		if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
			printf("ERROR: dup2 error to stdout");
		if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
			printf("ERROR: dup2 error to stderr");
		if (fds != STDIN_FILENO && fds != STDOUT_FILENO && fds != STDERR_FILENO)
			close(fds);
		return(0);		/* child returns 0 just like fork() */
	}
	else 
	{					/* parent */
		printf("[%d] pty fork parent\n", getpid());
		*ptrfdm = fdm;	/* return fd of master */
		return(pid);	/* parent returns pid of child */
	}
}


void kill_child()
{
	int tmp = pipe_child;
	pipe_child = 0;
	if (tmp != 0)
	{
		kill(tmp, SIGKILL);
	}
}

//Connect a new program with the new pty.
int start_pty(char **env, int fd, int mode)
{
	char buf[TTY_BUF_SIZE];
	int fdm;
	int pid, child;
	int nread, nwrite;
	int i;

	pid = pty_fork(&fdm, mode);

	signal(SIGCHLD, kill_child);


	if (pid == 0)
	{
		if (execv(env[0], env) < 0)
		{
			close(fd);
			perror("Error: ");
		}
		printf("[%d] Exec finish\n", getpid());
		exit(0);
	}
	else
	{
		child = fork();
		if (child == 0)
		{
			printf("[%d] pipe fork child\n", getpid());
			
			while (1)
			{
				memset(buf, 0, TTY_BUF_SIZE);
				if ((nread = read(fd, buf, TTY_BUF_SIZE)) < 0)
				{
					printf("ERROR: fail to read from stdin\n");
				}
				else if (nread == 0)
				{
					break;
				}
				printf("[%d] Send: ", getpid());
				for (i = 0; i<nread; i++)
				{
					printf("0x%02X ", buf[i]);
				}
				printf("\n");
				printf("Decode: %s\n", buf);
				if (write(fdm, buf, nread) != nread)
				{
					printf("ERROR: fail to write to fdm\n");
				}
			}
			printf("[%d] Read finish\n", getpid());
			close(fd);
			exit(0);
		}
		else
		{
			pipe_child = child;
			printf("[%d] pipe fork parent\n", getpid());
			while (1)
			{
				memset(buf, 0, TTY_BUF_SIZE);
				if ((nread = read(fdm, buf, TTY_BUF_SIZE)) <= 0)
				{
					break;
				}
				printf("[%d] Get: ", getpid());
				for (i = 0; i<nread; i++)
				{
					printf("0x%02X ", buf[i]);
				}
				printf("\n");
				printf("Decode: %s\n", buf);
				if (write(fd, buf, nread) != nread)
				{
					printf("ERROR: fail to write to stdout\n");
				}
			}
			printf("[%d] Write finish\n", getpid());
			close(fd);
			exit(0);
		}
	}

}
