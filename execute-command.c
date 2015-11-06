// UCLA CS 111 Lab 1 command execution
#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <unistd.h>

#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for waitpid
#include <unistd.h> // for execvp, fork
#include <stdio.h> // for fprintf
#include <stdlib.h> // for exit
#include <fcntl.h> // for file open constants


#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
static function definitions, etc.  */

int command_status (command_t c)
{
	return c->status;
}

void handle_io(command_t c)
{
	if (c->input != NULL) 
	{
		int readfd = open(c->input, O_RDONLY); //Stores file descript of c->input into readfd
		dup2(readfd, 0); //Copies readfd into standard input.
		close(readfd); //Close the no-longer needed file descriptor
	}

	if (c->output != NULL)
	{	//Will be able to read/write to file, file will have permission settings 777 when created
		int writefd = open(c->output, O_RDWR | O_TRUNC | O_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
		dup2(writefd, 1);
		close(writefd);
	}
}

void execute_command(command_t c)
{
	fprintf(stderr, "executeCHECKER %d\t", c->type);
	switch (c->type) {
	case SIMPLE_COMMAND: {	// i.e.  cat</etc/passwd Note: here the word stored is cat, *input stores /etc/passwd), tr a-z A-Z, g++ -c foo.c, echo sort failed!

			//fprintf(stderr, "\nSimple Command Running\n");
			int status;
			pid_t pid;			
			
			if ( (pid = fork()) == 0)  {	//This is the child process

				handle_io(c);
				//fprintf(stderr, "Child executing\n");
				status = execvp(c->u.word[0], c->u.word); //Executes the command. if execvp fails to execute the command it returns -1.
				c->status = WEXITSTATUS(status); //Store the status within the command
				//fprintf(stdout, "\nstatus: %d\n", c->status);
			}
			else 
			{ 	//parent process;
				waitpid(pid, &status, 0); //Wait for child process to run before continuing.
				printf("exit status: %d\n", WEXITSTATUS(status));
				c->status = WEXITSTATUS(status);
			}
			break;
		}
	case SUBSHELL_COMMAND: // ( A )
		{
			handle_io(c);
			execute_command(c->u.subshell_command);
			break;
		}
	case PIPE_COMMAND: 
		{ 	// A | B
			
			//printf("Pipe starting\n");
			int status;
			int pipefd[2];
			pid_t leftPid, rightPid, returnPid;

			pipe(pipefd); //pipefd[0] = read fd, pipefd[1] = write fd
			
			leftPid = fork();
			if ( leftPid == 0) //Child 1
			{
				//printf("leftPid start%d\n",leftPid);
								close(pipefd[0]); //Close unused fd within child
				dup2(pipefd[1], 1); // stores writefd into stdout
				execute_command(c->u.command[0]) ; //
				
				close(pipefd[1]);
				_exit(c->u.command[1]->status);
				
				//printf("leftPid end\n");
			}
			else //[Parent process
			{
				waitpid(-1, &status, 0);
				rightPid = fork();
				if (rightPid  == 0) // Child 2
				{
					//printf("rightPid start:%d\n", rightPid);
					close(pipefd[1]); //Close unused fd within child
					dup2(pipefd[0], 0);
					execute_command(c->u.command[1]);

					close(pipefd[1]);
					//printf("rightPid end\n");
					_exit(c->u.command[1]->status);
				}
				else { //Parent Process
					//printf("parent Start \n");
					close(pipefd[0]);
					close(pipefd[1]);
					
					waitpid(-1, &status, 0);
					
					
					//printf("WHAT PID IS THIS?: %d\n", returnPid);
					//I DONT KNOW WHERE MY KIDS ARE
					//WHERE ARE THEY?
					//HAAAAALP AHHHHHHHHH
					//if (returnPid == leftPid)
					//waitpid(-1, &status, 0); //Should be rightPid
					//else if (returnPid == rightPid)
					//	waitpid(-1, &status, 0);
					
					c->status = status;
					
					//printf("parent End \n");
					return;
				}
				
				break;
			}
			
			
			
		case AND_COMMAND: 		// A && B // Run B if A succeeded
			{
				int status;
				
				pid_t pid = fork();
				if (pid == 0) {
					execute_command(c->u.command[0]);
					//fprintf(stdout, "AND status: %d\n", c->u.command[0]->status);
					_exit(c->u.command[0]->status);
				}
				else
				{
					waitpid(pid, &status, 0);
					//fprintf(stdout, "Parent AND status: %d\n", status);
					if (status == 0)
					{
						execute_command(c->u.command[1]);
						_exit(c->u.command[1]->status);
					}
				}
				break;
			}
			
		case SEQUENCE_COMMAND:  // A ; B //Run A, then B
			{
				int status;
				pid_t pid = fork();
				if (pid == 0) {
					execute_command(c->u.command[0]);
					_exit(c->u.command[0]->status);
				}
				else{
					waitpid(pid, &status, 0);
					execute_command(c->u.command[1]);
					_exit(c->u.command[1]->status);
				}
				break;
			}
			
		case OR_COMMAND: 		// A || B  // Run B if A failed
			{
				int status;

				pid_t pid = fork();
				if (pid == 0) {
					execute_command(c->u.command[0]);
					//fprintf(stdout, "child OR status: %d\n", c->u.command[0]->status);
					_exit(c->u.command[0]->status);
				}
				else
				{
					waitpid(pid, &status, 0);
					//fprintf(stdout, "Parent OR status: %d\n", status);
					if (status != 0)
					{
						execute_command(c->u.command[1]);
						_exit(c->u.command[1]->status);
					}
				}
				break;
			}
		default:
			error (1, 0, "THIS MUST NOT BE A COMMAND");
			
		}
		//error (1, 0, "command execution not yet implemented");
	}
}
