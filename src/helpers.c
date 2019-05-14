// Your helper functions need to be here.
#include "shell_util.h"
#include "linkedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#ifdef DEBUG
	#define debug(fmt, ...) do{printf("DEBUG: %s:%s:%d " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
	#define info(fmt, ...) do{printf("INFO: %s:%s:%d " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#else
	#define debug(fmt, ...)
	#define info(fmt, ...) do{printf("INFO: " fmt, ##__VA_ARGS__);}while(0)
#endif

extern int conditional_flag;

int processComparator(void *process1, void *process2) {
	ProcessEntry_t* p1 = (ProcessEntry_t*) process1;
	ProcessEntry_t* p2 = (ProcessEntry_t*) process2;
	
	time_t* time1 = (time_t*) p1 -> seconds;
	time_t* time2 = (time_t*) p2 -> seconds;
	double result = difftime(*time1, *time2);
	
	if (result < 0)
		return -1;
	else if (result > 0)
		return 1;
	else
		return 0;
}


void sigint_handler() {
	debug("ctrl-c pressed\n");
}

void sigchild_handler() {
	debug("child process terminated: setting flag to 1\n");
	conditional_flag = 1;
	// int olderrno = errno;
	pid_t pid;
	if ((pid = wait(NULL)) < 0)
		debug("wait error\n");
	debug("pid: %d\n", (int) getpid());
}