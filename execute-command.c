// UCLA CS 111 Lab 1 command execution

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

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
	switch (c->type) {
		case SIMPLE_COMMAND: {	// i.e.  cat</etc/passwd Note: here the word stored is cat, *input stores /etc/passwd), tr a-z A-Z, g++ -c foo.c, echo sort failed!

			fprintf(stderr, "Simple Command Running\n");
			pid_t pid = fork();
			int status;	
			
			if (pid == 0)  {	//This is the child process
				

				if (c->input != NULL) {
					int readfd = open(c->input, O_RDONLY); //Stores file descript of c->input into readfd
					dup2(readfd, 0); //Copies readfd into standard input.
					close(readfd);
				}
				
				if (c->output != NULL)
				{	//Will be able to read/write to file, file will have permission settings 775
					int writefd = open(c->output, O_RDWR | O_TRUNC | O_CREAT | S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH
					dup2(writefd, 1);
					close(writefd);
				}
				
				
				fprintf(stderr, "Child executing\n");
				execvp(c->u.word[0], c->u.word); //Executes the command.
			}
			else { 	//parent process;
				waitpid(pid, &status, 0);
				printf("exit status: %d\n", WEXITSTATUS(status));
				c->status = WEXITSTATUS(status);
			}
			
			break;
		}
		case SUBSHELL_COMMAND: // ( A )
		
		break;
		
		case PIPE_COMMAND: { 	// A | B
			int pipefd[2] = pipe(pipefd); //pipefd[0] = read fd, pipefd[1] = write fd
			//pid_t leftPid, rightPid;

		}
		break;
		
		case AND_COMMAND: 		// A && B // Run B if A succeeded
		{
		}
		break;
		
		case SEQUENCE_COMMAND:  // A ; B //Run A, then B
		
		break;
		
		case OR_COMMAND: 		// A || B  // Run B if A failed
		
		break;
		
		
		
		
		
	}
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command execution not yet implemented");
}
