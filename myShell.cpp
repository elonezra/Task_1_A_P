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
#include <algorithm>
#define MAX_COMMANDS 20
using namespace std;

    char command[1024];
    char *token;
    char *outfile;
    char *buff;
    int i, fd, amper, redirect, redirect_err, redirect_append, retid, status;
    char *argv[10];
    char prompt[10] = "E_Shell2";
    char commands[MAX_COMMANDS][1024];
    int commands_index = 0;
    char skip_exec_flag = 0;
    int pipefd[2];
    int pid;

void redirection_check()
{
    //printf("redirect: %d  redirect_err: %d   redirect_append: %d\n", redirect, redirect_err, redirect_append);
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

void echo_command(char **arg)
{ 
    int t = 1;
    for (; arg[t] != NULL; t++)
    {
        printf("%s", arg[t]);
    }
        printf("\n");

}

int history_up_handel(string arg)
{
    int index = std::count(arg.begin(), arg.end(), '\033');

    if (index < commands_index)
    {
       return commands_index - index;
    }
    return commands_index-1;
    
}

/**
*   Define the signal handler function
*/
void signalHandler(int signum) 
{
    printf("You typed Control-C! write `quit` to exit\n");
    fflush(stdout);
}

void cmd_parser(char *cmd)
{

    strcpy(commands[commands_index % MAX_COMMANDS], cmd);
    commands_index++;
    /* parse command line */
    i = 0;
    token = strtok (cmd," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;
}

void cd_command(char* dir)
{
    if (chdir(dir) == -1)
        perror("cd");
}

/**
* Method used to manage pipes.
*/ 
void pipeHandler(char * args[]){
	// File descriptors
	int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
	int filedes2[2];
	
	int num_cmds = 0;
	
	char *command[256];
	
	pid_t pid;
	
	int err = -1;
	int end = 0;
	
	// Variables used for the different loops
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	
	// First we calculate the number of commands (they are separated
	// by '|')
	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;
	
	// Main loop of this method. For each command between '|', the
	// pipes will be configured and standard input and/or output will
	// be replaced. Then it will be executed
	while (args[j] != NULL && end != 1){
		k = 0;
		// We use an auxiliary array of pointers to store the command
		// that will be executed on each iteration
		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				// 'end' variable used to keep the program from entering
				// again in the loop when no more arguments are found
				end = 1;
				k++;
				break;
			}
			k++;
		}
		// Last position of the command will be NULL to indicate that
		// it is its end when we pass it to the exec function
		command[k] = NULL;
		j++;		
		
		// Depending on whether we are in an iteration or another, we
		// will set different descriptors for the pipes inputs and
		// output. This way, a pipe will be shared between each two
		// iterations, enabling us to connect the inputs and outputs of
		// the two different commands.
		if (i % 2 != 0){
			pipe(filedes); // for odd i
		}else{
			pipe(filedes2); // for even i
		}
		
		pid=fork();
		
		if(pid==-1){			
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]); // for odd i
				}else{
					close(filedes2[1]); // for even i
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){
			// If we are in the first command
			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}
			// If we are in the last command, depending on whether it
			// is placed in an odd or even position, we will replace
			// the standard input for one pipe or another. The standard
			// output will be untouched because we want to see the 
			// output in the terminal
			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){ // for odd number of commands
					dup2(filedes[0],STDIN_FILENO);
				}else{ // for even number of commands
					dup2(filedes2[0],STDIN_FILENO);
				}
			// If we are in a command that is in the middle, we will
			// have to use two pipes, one for input and another for
			// output. The position is also important in order to choose
			// which file descriptor corresponds to each input/output
			}else{ // for odd i
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{ // for even i
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}		
		}
				
		// CLOSING DESCRIPTORS ON PARENT
		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){					
				close(filedes[0]);
			}else{					
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){					
				close(filedes2[0]);
				close(filedes[1]);
			}else{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
				
		waitpid(pid,NULL,0);
				
		i++;	
	}
}

/**
 * check if input contain |
*/
bool containsPipe(char* arr[]) { 
    int i = 0; 
    while (arr[i] != nullptr) 
    { 
        if (strcmp(arr[i], "|") == 0) { return true; } 
        i++; 
    } 
    return false; 
    }
int main() {

while (1)
{
    signal(SIGINT, signalHandler);
    skip_exec_flag = 0;
    printf("%s: ", prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    

    if (!strcmp(command, "!!"))
        {
            if (commands_index == 0)
            {
                printf("No previous command.\n");
                continue;
            }
            else
            {
                /* Get last command and execute it */
                strcpy(command, commands[(commands_index - 1) % MAX_COMMANDS]);
                printf("Executing: %s\n", command);
            }
        }
    else{
        if (strncmp(command, "\033[A", 4) == 0)
        {
            std::string mystring(command);
            if(commands_index==0)
            {
                continue;
            }
            strcpy(command, commands[(history_up_handel(command))]);
        }
    }
        cmd_parser(command);
    // int j = 0;
    // while(i > j)
    // {
    //     printf("%d) %s\n",j, argv[j]);
    //     j++;
    // }

  
    
    /* Is command empty */
    if (argv[0] == NULL)
        continue;


    if (!strcmp(argv[0], "echo"))
    {
        echo_command(argv);
        continue;
    }
    if (containsPipe(argv))
    {
    
        pipeHandler(argv);
        continue;
    }
    
    if (!strcmp(argv[0], "quit"))
    {
        exit(0);
    }

    if (!strcmp(argv[0], "cd"))
    {
        cd_command(argv[1]);
    }
    
    
    if (argv[i-2] != NULL && !strcmp(argv[i - 2], "=") && !strcmp(argv[0], "prompt"))
    {
        strncpy(prompt, argv[i - 1], sizeof(prompt));
    }

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
        if(!skip_exec_flag){
            signal (SIGINT, SIG_DFL);
            execvp(argv[0], argv); 
        }
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
