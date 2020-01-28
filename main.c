#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <signal.h>

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

    uinput = strtok(buf, " ");

    argc = 0;
    while (uinput != NULL && argc < MAXARGS && strcmp(uinput, "\n") != 0)
    {
        char * newline = strchr(uinput, '\n'); // remove newline char
        if (newline != NULL) *newline = '\0';
            
        if (argc == 0)
        {
            int blen = strlen("/bin/");
            args[argc] = malloc((strlen(uinput)+blen) * sizeof(char));
            strcpy(args[argc], "/bin/");
            strcpy(args[argc]+blen, uinput);
        }
        else 
        {
            args[argc] = malloc(strlen(uinput) * sizeof(char));
            strcpy(args[argc], uinput);
        }
        ++argc;
        uinput = strtok(NULL, " ");

    }
    args[argc] = NULL;

    identifyproc(args, argc);
}

void identifyproc(char ** args, int argc)
{
    //BACKGROUND PROCESS
    if (strcmp(args[argc-1], "&") == 0)
    {
        args[argc-1] = NULL;
        background(args, argc);
    }
    
    //FOREGROUND PROCESS
    else
    {
        foreground(args, argc);
    }
}

void foreground(char ** args, int argc)
{
    int pid = fork();

    //CHILD PROCESS
    if (pid == 0) 
    {
        execv(args[0], args);
        exit(0);
    }
    wait(NULL);
}

void sig_handler(int sig)
{
    printf("Inside signal handler\n");
    exit(0);
}

void background(char ** args, int argc)
{
    int pid = fork();

    signal(SIGINT, sig_handler);
    //CHILD PROCESS
    if (pid == 0)
    {
        execv(args[0], args);
    }
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
