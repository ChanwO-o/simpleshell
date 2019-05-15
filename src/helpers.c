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

extern int errno;
extern int conditional_flag;
extern int childcount;

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

void addBackProcess(char* buffer, List_t* bg_list) {
	// debug("***buffer:  %s\n", buffer);
	childcount++; // increase child counter
	debug("New child process; childcount: %d\n", childcount);
	
	// create linkedlist node
	struct ProcessEntry* processentry = (struct ProcessEntry*) malloc(sizeof(struct ProcessEntry));
	// debug("1\n");
	processentry -> cmd = buffer;
	processentry -> pid = getpid();
	processentry -> seconds = time(NULL);
	
	// debug("cmd: %s\n", processentry -> cmd);
	// debug("pid: %d\n", processentry -> pid);
	// debug("seconds: %d\n", (int) processentry -> seconds);
	
	node_t node;
	node.value = processentry;
	insertRear(bg_list, &node);
	// printList(bg_list, INT_MODE);
}

// void removeBackProcess(pid_t pid, List_t* bg_list) {
	// // int removeByPid(List_t* list, pid_t p);
// }

int isBackgroundProcess(pid_t pid, List_t* bg_list) {
	
}

void sigint_handler() {
	debug("ctrl-c pressed\n");
}

void sigchild_handler() {
	debug("child process terminated: setting flag to 1\n");
	
	// check if current process is a background process (?) (in linkedlist)
	
	
	conditional_flag = 1; // set flag 1 to denote that there's a terminated child
	// pid_t pid;
	// int status;
	// pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
	// debug("pid: %d\n", pid);
	// while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
		// debug("pid: %d\n", pid);
		// childcount--;
		// debug("reaped child pid: %d\n", pid);
	// }
	// debug("reaped child pid: %d\n", (int) getpid());
	// removeByPid();
}