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

int conditional_flag = 0; // flag denoting that there's a terminated child process (1) or none (0)

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
			debug("flag is 1; check if this is background or not\n");
			// get the ProcessEntry object for this pid; if this is foreground, will not be in the ll and should return NULL
			ProcessEntry_t* processentry = getByPid(pid, &bg_list);
			if (processentry != NULL) {
				debug("process is background; remove from ll\n");
				removeByPid(&bg_list, pid);
				printf(BG_TERM, pid, processentry -> cmd);
			}
			else
				debug("process is foreground; ignore\n");
			
			debug("new length of list: %d\n", bg_list.length);
			
			conditional_flag = 0; // no more zombies left to terminate
			// debug("zombies cleared, flag set back to 0\n");
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
			free(fullbuffer);
			break;
		}

		// Remove newline character from buffer, if it's there
		if(buffer[nbytes - 1] == '\n')
			buffer[nbytes- 1] = '\0';
		
		char s[100];
		
		// Handling empty strings
		if(strcmp(buffer, "") == 0) {
			free(buffer);
			free(fullbuffer);
			continue;
		}
		
		// Parsing input string into a sequence of tokens
		size_t numTokens;
		*args = NULL;
		numTokens = tokenizer(buffer, args);

		// handle 0 args (whitespace input)
		if (numTokens == 0) {
			free(buffer);
			free(fullbuffer);
			continue;
		}
		
		// Terminating the shell
		if(strcmp(args[0],"exit") == 0) {
			killAllBackgrounds(&bg_list);
			free(buffer);
			free(fullbuffer);
			return 0;
		}
		
		// handle estatus command
		if (strcmp(args[0], "estatus") == 0) {
			printf("%d\n", exit_status);
			free(buffer);
			free(fullbuffer);
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
			free(fullbuffer);
			continue;
		}
		
		// handle redirection: only necessary when there's at least 3 tokens
		int inputfd, outputfd, stderrfd, appendfd; // file descriptors for using in child
		int opinput = -1; int opoutput = -1; int opstderr = -1; int opappend = -1; // index of args where each redirect symbol occurs ( <  >  2>  >> )
		if (numTokens >= 3)
		{
			// printf("current pid: %d\n", pid);
			int i;
			for (i = 1; i < numTokens - 1; ++i) {
				if (strcmp(args[i], "<") == 0) {
					opinput = i;
					debug("stdin from: %s\n", args[i + 1]);
				}
				if (strcmp(args[i], ">") == 0) {
					opoutput = i;
					debug("stdout to: %s\n", args[i + 1]);
				}
				if (strcmp(args[i], "2>") == 0) {
					opstderr = i;
					debug("stderr to: %s\n", args[i + 1]);
				}
				if (strcmp(args[i], ">>") == 0) {
					opappend = i;
					debug("append to: %s\n", args[i + 1]);
				}
			}
			char c;
			mode_t mode =  S_IRUSR;
			if (opinput != -1) {
				debug("aa\n");
				inputfd = open(args[opinput + 1], O_RDONLY, mode);
			}
			if (opoutput != -1)
				outputfd = open(args[i + 1], O_WRONLY);
			if (opstderr != -1)
				stderrfd = open(args[i + 1], O_WRONLY);
			if (opappend != -1)
				appendfd = open(args[i + 1], O_APPEND);
			
		}
		
		
		pid = fork(); // if not a built-in command, fork and run the command on a child process (as to not clog up parent)
		
		
		// handle background process
		if (strcmp(args[numTokens - 1], "&") == 0) {
			if (pid != 0) {
				// ONLY FOR THE PARENT PROCESS: create a ProcessEntry node and add to ll with the pid of the child process that was just created
				addBackProcess(fullbuffer, &bg_list, pid);
			}
		}
		
		if (pid == 0){ //If zero, then it's the child process
			//read(inputfd, &c, 0);
			if (opinput != -1) {
				if (dup2(inputfd, STDIN_FILENO) < 0)
					perror("error dupin\n");
				if (close(STDIN_FILENO) < 0)
					perror("error closing stdin\n");
				if (close(inputfd) < 0)
					perror ("error closing inputfd\n");
			}
			if (opoutput != -1)
				outputfd = open(args[i + 1], O_WRONLY);
			if (opstderr != -1)
				stderrfd = open(args[i + 1], O_WRONLY);
			if (opappend != -1)
				appendfd = open(args[i + 1], O_APPEND);
			
			// perror ("aaaa\n");
			exec_result = execvp(args[0], &args[0]);
			// perror ("bbbb\n");
			if(exec_result == -1){ //Error checking
				// perror ("cccc\n");
				printf(EXEC_ERR, args[0]);
				// perror ("dddd\n");
				exit(EXIT_FAILURE);
			}
			perror ("eeee\n");
		    exit(EXIT_SUCCESS);
			perror ("ffff\n");
		}
		else{ // Parent Process
			if (strcmp(args[numTokens - 1], "&") == 0) {
				// if background cmd, just ignore since the child process is handlilng it anyways. This allows user to keep using the shell without waiting
			}
			else { // else, wait for the process
				wait_result = waitpid(pid, &exit_status, 0);
			}
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
		free(fullbuffer);
	}
	return 0;
}

