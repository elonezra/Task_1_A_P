#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <iostream>
using namespace std;

int main() {
char command[1024];
char *token;
char *outfile;
char *buff;
int i, fd, amper, redirect, redirect_append, retid, status;
char *argv[10];

while (1)
{
    printf("hello: ");
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;
    int j = 0;
    while(i >j)
    {
        printf("%d) %s\n",j, argv[j]);
        j++;
    }
    cerr << strcmp(argv[i - 2], ">") << "\n";
    cerr << strcmp(argv[i - 2], ">>") << "\n";
    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    if (! strcmp(argv[i - 2], ">")) {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }
    else 
        redirect = 0; 

    if (argv[i - 2] != NULL && ! strcmp(argv[i - 2], ">>")) {//cant cmpr null
        //printf("found >>\n");
        redirect_append = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }
    else 
        redirect_append = 0; 

    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO); 
            dup(fd);
            close(fd); 
        }
        else if(1)
        {
                        fd = creat(outfile, 0660); 

            close(STDERR_FILENO);
            dup(fd);
        }
        else if (redirect_append)
        {
            fd = creat(outfile, O_WRONLY | O_APPEND); 
            close (STDOUT_FILENO); 
            read(STDOUT_FILENO, buff, 100);
            write(fd, buff, strlen(buff));
            close(fd); 
        }
         
        
        execvp(argv[0], argv);
 
        
        
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
