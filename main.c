#include <stdio.h>
#include <string.h>
#include <unistd.h> 

#define MAXARGS 11
#define ARGMAXLEN 10

struct command
{
    char cmd[80];
    char args[80];
};

void createcmd(char * buf, struct command * c)
{
    char * uinput;
    int len;
    int index = 0;
    
    uinput = strtok(buf, " ");
    strcpy(c->cmd, uinput);
  
    while (uinput != NULL)
    {
        uinput = strtok(NULL, " ");
        if (uinput != NULL)
        {
            len = strlen(uinput);
            strcpy(c->args+index, uinput);
            index += len;
            c->args[index] = ' ';
            index++;
        }
    }
    c->args[index-2] = '\0';
}

void parsecmd(char * buf)
{
    char * uinput;
    char * args[MAXARGS]; 
    unsigned int argc;

    uinput = strtok(buf, " ");

    argc = 0;
    while (uinput != NULL && argc < MAXARGS)
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

    execproc(args);
}

void execproc(char ** args)
{    
    int pid;
    
    printf("")


    if ((pid = fork()) == 0) { // process is child
        printf("child\n");
        execv(args[0], args);
        // execv("/bin/ls", args);
        printf("child finished\n");
    }
    else { // process is parent
        printf("parent\n");
        printf("parent finished\n");
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
            parsecmd(buf);

            /*
            char * args[MAXARGS]; 
            unsigned int argc;

            uinput = strtok(buf, " ");

            argc = 0;
            while (uinput != NULL && argc < MAXARGS)
            {
				char * newline = strchr(uinput, '\n'); // remove newline char
				if (newline != NULL) *newline = '\0';
					
				if (argc == 0)
				{
					args[argc] = malloc(ARGMAXLEN * sizeof(char));
					strcpy(args[argc], "/bin/");
					int len = strlen("/bin/");
					strcpy(args[argc]+len, uinput);
					++argc;
					uinput = strtok(NULL, " ");
				}
				else
				{
					args[argc] = malloc(ARGMAXLEN * sizeof(char));
					strcpy(args[argc], uinput);
					argc++;
					uinput = strtok(NULL, " ");
				}
            }
            args[argc] = NULL;
			
			int pid;
			if ((pid = fork()) == 0) { // process is child
				printf("child\n");
				execv(args[0], args);
				// execv("/bin/ls", args);
				printf("child finished\n");
			}
			else { // process is parent
				printf("parent\n");
				execv(args[0], args);
				// execv("/bin/ls", args);
				printf("parent finished\n");
			}
            */

            break; 

            //struct command c;
            //createcmd(buf, &c);
            
            /*
            if (strcmp(c.cmd, "quit\n") == 0)
            {
                break;
            }
            */
        }
    }

    return 0;
}
