#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <signal.h>
#include <stdlib.h>

#define MAXARGS 11
#define ARGMAXLEN 10

struct command
{
    char cmd[80];
    char args[80];
};

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
            int blen = strlen("/bin/");
            args[argc] = malloc((strlen(uinput)+blen)*sizeof(char));
            strcpy(args[argc], "/bin/");
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
    }
    wait(NULL);
}

void background(char ** args, int argc, char * outFile, char * inFile)
{
    int pid = fork();

    signal(SIGINT, sigint_handler);
	signal(SIGCHLD, sigchld_handler);
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
    }
}

void sigint_handler(int sig)
{
    printf("sigint handler: a process was interrupted\n");
    exit(0);
}

void sigchld_handler(int sig)
{
	printf("sigchild handler: a child process was terminated\n");
}

int main()
{
    char buf[80];
    char * uinput;
    
    while (1)
    {
        printf("prompt>");

        if (fgets(buf, sizeof(buf), stdin) != NULL)
        { 
            if (strcmp(buf, "quit\n") == 0)
                break;
            parsecmd(buf);       
        }
    }
    return 0;
}
