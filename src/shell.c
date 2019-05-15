#include "shell_util.h"
#include "linkedList.h"
#include "helpers.h"

// Library Includes
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

int errno = 0;
int conditional_flag = 0; // flag denoting that there's a terminated child process (1) or none (0)
int childcount = 0; // number of active child processes alive

int main(int argc, char *argv[])
{
	int i; //loop counter
	char *args[MAX_TOKENS + 1];
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
    List_t bg_list;

    //Initialize the linked list
    bg_list.head = NULL;
    bg_list.length = 0;
    bg_list.comparator = processComparator;  // Don't forget to initialize this to your comparator!!!

	// Setup segmentation fault handler
	if(signal(SIGSEGV, sigsegv_handler) == SIG_ERR)
	{
		perror("Failed to set signal handler");
		exit(-1);
	}
	// setup Ctrl-C handler
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
	{
		perror("Failed: ctrl-c handling\n");
		exit(-1);
	}
	// setup child termination handler
	if (signal(SIGCHLD, sigchild_handler) == SIG_ERR)
	{
		perror("Failed: sigchld handling\n");
		exit(-1);
	}
	
	while(1) {
		// check if a background child process was terminated. if 1, remove the process node from linkedlist
		if (conditional_flag == 1) {
			debug("flag is 1; traverse through ll and remove the terminated child process\n");
			
			// printList(&bg_list, STR_MODE);
			debug("length of list: %d\n", bg_list.length);
			removeByPid(&bg_list, getpid());
			debug("new length of list: %d\n", bg_list.length);
			// removeTerminatedFromList(&bg_list);
			
			
			// int status;
			// pid_t pid_temp;
			// while ((pid_temp = waitpid(pid, &status, WNOHANG | WUNTRACED)) > 0)
			// {
				// debug("child pid: %d\n", pid_temp);
				// if (WIFEXITED(status)) {
					// debug("child terminated normally; now removing child's pid: %d\n", pid_temp);
					// removeByPid(&bg_list, pid_temp);
					
					// fprintf(stdout, BG_TERM);
				// }
			// }
			// debug("pid: %d\n", pid_temp);
			
			conditional_flag = 0; // no more zombies left to terminate
			debug("zombies cleared, flag set back to 0\n");
		}
		
		// DO NOT MODIFY buffer
		// The buffer is dynamically allocated, we need to free it at the end of the loop
		char * const buffer = NULL;
		size_t buf_size = 0;

		// Print the shell prompt
		display_shell_prompt();
		
		// Read line from STDIN
		ssize_t nbytes = getline((char **)&buffer, &buf_size, stdin);
		char* fullbuffer = (char*) malloc(50 * sizeof(char)); // store full cmd buffer before it gets tokenized
		strcpy(fullbuffer, buffer);
		
		// No more input from STDIN, free buffer and terminate
		if(nbytes == -1) {
			free(buffer);
			break;
		}

		// Remove newline character from buffer, if it's there
		if(buffer[nbytes - 1] == '\n')
			buffer[nbytes- 1] = '\0';
		
		char s[100];
		// debug("%s\n", getcwd(s, 100));
		
		// Handling empty strings
		if(strcmp(buffer, "") == 0) {
			free(buffer);
			continue;
		}
		
		// Parsing input string into a sequence of tokens
		size_t numTokens;
		*args = NULL;
		numTokens = tokenizer(buffer, args);

		// handle 0 args (whitespace input)
		if (numTokens == 0) {
			free(buffer);
			continue;
		}
		
		if(strcmp(args[0],"exit") == 0) {
			// Terminating the shell
			free(buffer);
			return 0;
		}
		
		// handle estatus command
		if (strcmp(args[0], "estatus") == 0) {
			printf("%d\n", exit_status);
			free(buffer);
			continue;
		}
		
		// handle cd command
		if (strcmp(args[0], "cd") == 0) {
			if (numTokens == 1) // only 'cd': go to home dir
				if (chdir(getenv("HOME")) == 0) // cd success: print current path
					printf("%s\n", getcwd(s, 100));
				else
					fprintf(stderr, DIR_ERR);
			else {
				if (chdir(args[1]) == 0) // cd success: print current path
					printf("%s\n", getcwd(s, 100));
				else
					fprintf(stderr, DIR_ERR);
			}
			free(buffer);
			continue;
		}
		
		// handle redirection
		// int i;
		// for (i = 1; i < numTokens; ++i) {
			// if (strcmp(args[i], "<") == 0) {
				// debug("stdin from: %s\n", args[i + 1]);
				
			// }
			// if (strcmp(args[i], ">") == 0) {
				// debug("stdout to: %s\n", args[i + 1]);
			// }
			// if (strcmp(args[i], "2>") == 0) {
				// debug("stderr to: %s\n", args[i + 1]);
			// }
			// if (strcmp(args[i], ">>") == 0) {
				// debug("append to: %s\n", args[i + 1]);
			// }
		// }
		
		// handle background process
		if (strcmp(args[numTokens - 1], "&") == 0) {
			addBackProcess(fullbuffer, &bg_list);
		}
		
		pid = fork(); // if not a built-in command, fork and run the command on a child process (as to not clog up parent)
		
		
		if (pid == 0){ //If zero, then it's the child process
			// debug("child\n");
			
			
			exec_result = execvp(args[0], &args[0]);
			if(exec_result == -1){ //Error checking
				printf(EXEC_ERR, args[0]);
				exit(EXIT_FAILURE);
			}
		    exit(EXIT_SUCCESS);
		}
		else{ // Parent Process
			// debug("parent\n");
			wait_result = waitpid(pid, &exit_status, 0);
			// debug("wait_result: %d\n", wait_result);
			if(wait_result == -1){
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
			}
		}
		
		
		// handle exit status
		if (exit_status != 0)
			exit_status = 1;

		// Free the buffer allocated from getline
		free(buffer);
	}
	return 0;
}

