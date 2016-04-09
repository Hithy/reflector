#include "misc.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

time_t rawtime;
struct tm *timeinfo;

char log_name[256] = { 0 };

char * get_actime()
{
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
}


int log_init(char *logfile)
{
	if (logfile == NULL || strlen(logfile) > 255)
		return -1;
	memset(log_name, 0, 256);
	memcpy(log_name, logfile, strlen(logfile));
	return 0;
}

void write_log(char *str)
{
	if (log_name[0] == 0)
	{
		printf("You should init log firstly\n");
		return;
	}
	if (str == NULL)
	{
		printf("Invalid string to log\n");
		return;
	}

	FILE *fg = fopen(log_name, "a");

	if (fg == NULL)
	{
		printf("Fail to open file\n");
		return;
	}

	printf("[%d] Loging...\n%s\n", getpid(), str);

	fputs(str, fg);
	fclose(fg);
}