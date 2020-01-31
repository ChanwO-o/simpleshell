#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXARGS 11
#define MAXBGPROCESSES 25

pid_t bgprocesses[MAXBGPROCESSES];
int bgprocesses_count = 0;

void parsecmd(char * buf)
{
    char * uinput;
    char * args[MAXARGS]; 
    unsigned int argc;
    char ofname[50];
    char ifname[50];
    char * outFile = NULL;
    char * inFile = NULL;

    uinput = strtok(buf, " ");

    argc = 0;
    while (uinput != NULL && argc < MAXARGS && strcmp(uinput, "\n") != 0 &&  
            strcmp(uinput, ">") != 0 && strcmp(uinput, "<") != 0)
    {
        char * newline = strchr(uinput, '\n'); // remove newline char
        if (newline != NULL) *newline = '\0';
            
        if (argc == 0)
        {
            int blen = strlen("./executables/");
            args[argc] = malloc((strlen(uinput)+blen)*sizeof(char));
            strcpy(args[argc], "./executables/");
            strcpy(args[argc]+blen, uinput);
        }
        else
        {
            args[argc] = malloc(strlen(uinput)*sizeof(char));
            strcpy(args[argc], uinput);
        }
        uinput = strtok(NULL, " ");
        ++argc;
    } 
    while (uinput != NULL)
    {
        char * newline = strchr(uinput, '\n'); // remove newline char
        if (newline != NULL) *newline = '\0';

        if (strcmp(uinput, ">") == 0)
        {
            uinput = strtok(NULL, " ");
            char * newline = strchr(uinput, '\n'); // remove newline char
            if (newline != NULL) *newline = '\0';
            strcpy(ofname, uinput);
            outFile = ofname;
        }
        else if (strcmp(uinput, "<") == 0)
        {
            uinput = strtok(NULL, " ");
            char * newline = strchr(uinput, '\n'); // remove newline char
            if (newline != NULL) *newline = '\0';
            strcpy(ifname, uinput);
            inFile = ifname;
        }
        else if (strcmp(uinput, "&") == 0)
        {
            args[argc] = malloc(strlen(uinput)*sizeof(char));
            strcpy(args[argc], uinput);
            ++argc;
        }
        uinput = strtok(NULL, " ");
    }
    args[argc] = NULL;
    identifyproc(args, argc, outFile, inFile);
}

void identifyproc(char ** args, int argc, char * outFile, char * inFile)
{
    //BACKGROUND PROCESS
    if (strcmp(args[argc-1], "&") == 0)
    {
        args[argc-1] = NULL;
        background(args, argc, outFile, inFile);
    }
    
    //FOREGROUND PROCESS
    else
    {
        foreground(args, argc, outFile, inFile);
    }
}

void outputredir(char * outFile)
{
    FILE * file = fopen(outFile, "w");
    int fd = fileno(file);
    close(1);
    dup(fd);
    close(fd);
}

void inputredir(char * inFile)
{
    FILE * file = fopen(inFile, "r");
    int fd = fileno(file);
    close(0);
    dup(fd);
    close(fd);
}

void foreground(char ** args, int argc, char * outFile, char * inFile)
{
    int pid = fork();

    //CHILD PROCESS
    if (pid == 0) 
    {
        if (outFile != NULL)
        {
            outputredir(outFile);               
        }
        if (inFile != NULL)
        {
            inputredir(inFile);
        }
        execv(args[0], args);
        exit(0);
    }
    wait(NULL);
}

void addbackgroundprocess(int pid)
{
	int i;
	for (i = 0; i < 25; ++i) // store pid in first empty slot
	{
		if (bgprocesses[i] == NULL)
		{
			printf("addbackgroundprocess(): adding pid %d to bg list at i: %d\n", pid, i);
			bgprocesses[i] = pid;
			bgprocesses_count++;
			break;
		}
	}
}

void removebackgroundprocess(pid_t* pid)
{
	int i;
	for (i = 0; i < 25; ++i)
	{
		if (bgprocesses[i] == pid)
		{
			printf("removebackgroundprocess(): removing pid %d from bg list at i: %d\n", pid, i);
			bgprocesses[i] = NULL;
			bgprocesses_count--;
			break;
		}
	}
}

void sigint_handler(int sig)
{
    printf("sigint handler: a background process was interrupted\n");
    exit(0);
}

void sigchld_handler(int sig)
{
	int i, status, terminated;
	for (i = 0; i < 25; ++i)
	{
		terminated = waitpid(bgprocesses[i], &status, WNOHANG); // try waiting for each pid registered; if not hanging, will pass.
		if (terminated != -1)
		{
			printf("sigchild handler: SUCCESS! terminated PID: %d\n", terminated);
			removebackgroundprocess(terminated);
			break;
		}
	}
}

void background(char ** args, int argc, char * outFile, char * inFile)
{
	signal(SIGCHLD, sigchld_handler);
    int pid = fork();

    //CHILD PROCESS
    if (pid == 0)
    {
        if (outFile != NULL)
        {
            outputredir(outFile);
        }
        if (inFile != NULL)
        {
            inputredir(inFile);
        }
        execv(args[0], args);
        exit(0); // exit child processes that reach here (i.e. when bad cmd issued to execv)
    }
	else
	{
		addbackgroundprocess(pid);
	}
}

int main()
{
    char buf[80];
    char * uinput;
    
    while (1)
    {
        printf("prompt> bg_processes: %d ", bgprocesses_count);

        if (fgets(buf, sizeof(buf), stdin) != NULL)
        { 
            if (strcmp(buf, "quit\n") == 0)
                break;
            
            else if (buf[0] != '\n')
                parsecmd(buf);       
        }
    }
    return 0;
}
