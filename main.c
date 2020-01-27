#include <stdio.h>
#include <string.h>

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

int main()
{
    char buf[80];
    char * uinput;
    
    while (1)
    {
        printf("prompt>");

        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            char ** argv;
            char * args[MAXARGS]; 
            unsigned int argc;

            uinput = strtok(buf, " ");

            argc = 0;
            while (uinput != NULL && argc < MAXARGS)
            {
                args[argc] = malloc(ARGMAXLEN * sizeof(char));
                strcpy(args[argc], uinput);
                argc++;
                uinput = strtok(NULL, " ");
            }
            args[argc] = NULL;

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
