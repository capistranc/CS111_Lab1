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
void find_dependency(int** dependency_graph, int* has_dependency, int num_cmds) {
	//generate a 1 dimensional array describing whether processes have any dependencies
	int i;
	int j;
	for (i = 0; i<num_cmds; i++) {
		has_dependency[i] = 0;
		for (j = 0; j<num_cmds; j++) {
			if (dependency_graph[i][j] == 1) {
				has_dependency[i] = 1;
			}
		}
	}
}

void run_ready_commands(command_t* trees, int** dependency_graph, int* has_dependency, int* executable, int num_cmds)
{
	//run commands with no dependency that are executable, i.e. not already running or not completed
	int i;
	int j;
	int cmd_idx;
	for (cmd_idx = 0; cmd_idx < num_cmds; cmd_idx++) {
		if(has_dependency[cmd_idx] == 0 && executable[cmd_idx] == 1) {
			pid_t pid = fork();
			if (pid == 0) {
				//make this command non-executable because we are about to run it
				executable[cmd_idx] = 0;
				execute_command(trees[cmd_idx]);
				//finished executing command so we update the dependency graph to remove any dependencies on it
				for (i = 0; i < num_cmds; i++) {
					for (j = 0; j < num_cmds; j++) {
						if (i == cmd_idx || j == cmd_idx) {
							dependency_graph[i][j] == 0;
						}
					}
				}
				//now we update our dependency indicator using our updated dependency graph
				find_dependency(dependency_graph, has_dependency, num_cmds);
				//finally we call this function recursively to execute any executable commands who have had their dependencies resolved
				run_ready_commands(trees, dependency_graph, has_dependency, executable, num_cmds);
			} else {
				continue; //parent so continue making children
			}
		}
	}
}

void executeTimeTravel(command_stream_t command_stream, int** dependency_graph)
{
	int num_cmds = length_command_stream(command_stream);
	int* has_dependency = (int*)malloc(sizeof(int)*num_cmds);
	int* executable = (int*)malloc(sizeof(int)*num_cmds);
	command_t* trees = (command_t*)malloc(sizeof(command_t)*num_cmds);
	int cmd_idx = 0;
	command_t command;
	while ((command = read_command_stream(command_stream))) {
		trees[cmd_idx] = command;
	}
	find_dependency(dependency_graph, has_dependency, num_cmds);
	run_ready_commands(trees, dependency_graph, has_dependency, executable, num_cmds);
}

	
