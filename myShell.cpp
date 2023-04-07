#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

    char command[1024];
    char *token;
    char *outfile;
    char *buff;
    int i, fd, amper, redirect, redirect_err, redirect_append, retid, status;
    char *argv[10];


void redirection_check()
{
    printf("redirect: %d  redirect_err: %d   redirect_append: %d\n", redirect, redirect_err, redirect_append);
    std::cout.flush();// flush the buffer so my prints wont get in the files
    if (redirect) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO); 
            dup(fd);
            close(fd); 
        }
        else if(redirect_err)
        {
            fd = creat(outfile, 0660); 
            close(STDERR_FILENO);
            dup(fd);
            close(fd);
        }
        else if (redirect_append)
        {
            fd=open(outfile ,O_CREAT | O_RDWR | O_APPEND,0666);
            dup2(fd,STDOUT_FILENO);
            close(fd); 
        }
}

int main() {
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
    while(i > j)
    {
        printf("%d) %s\n",j, argv[j]);
        j++;
    }

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

    if (argv[i - 2] != NULL && ! strcmp(argv[i - 2], "2>")) {//added the left cuse can't comper null
        redirect_err = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }
    else {
        redirect_err = 0; 
        if (argv[i - 2] != NULL && ! strcmp(argv[i - 2], ">")) {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
            }
        else 
            redirect = 0; 
    }
    if (argv[i - 2] != NULL && ! strcmp(argv[i - 2], ">>")) {//added the left cuse can't comper null
        redirect_append = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }
    else 
        redirect_append = 0; 

    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO ? */
        redirection_check();
 
        execvp(argv[0], argv); 
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
