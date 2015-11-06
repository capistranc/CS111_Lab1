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


void executeTimeTravel(command_stream_t command_stream, int** m)
{
	int count = 0;
	
	
	//printMatrix(graph->dep_matrix, graph->len);
	
	error(1,0,"Not implemented yet\n");
}

	
