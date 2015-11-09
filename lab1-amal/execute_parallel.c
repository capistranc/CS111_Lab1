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

#include <pthread.h>

#include <error.h>

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
	/*printf("dependency: ");
	for (i = 0; i < num_cmds; i++) {
	  printf("%d", has_dependency[i]);
	}
	printf("\n");*/
}

struct execute_wrapper_arg {
  int cmd_idx;
  int num_cmds;
  int ** dependency_graph;
  int *has_dependency;
  command_t cmd;
  int * completed;
};

void *execute_wrapper(void* arg) {
  int cmd_idx = ((struct execute_wrapper_arg*)arg)->cmd_idx;
  int num_cmds = ((struct execute_wrapper_arg*)arg)->num_cmds;
  int ** dependency_graph = ((struct execute_wrapper_arg*)arg)->dependency_graph;
  int *has_dependency = ((struct execute_wrapper_arg*)arg)->has_dependency;
  command_t cmd = (command_t)(((struct execute_wrapper_arg*)arg)->cmd);
  int * completed = ((struct execute_wrapper_arg*)arg)->completed;
  
  printf("execute command %d\n", cmd_idx);
  execute_command(cmd);
  //printf("dependcency graph\n");
  int i;
  int j;
  for (i = 0; i < num_cmds; i++) {
    for (j = 0; j < num_cmds; j++) {
      if (i == cmd_idx || j == cmd_idx) {
	dependency_graph[i][j] = 0;
      }
      //printf("%d", dependency_graph[i][j]);
    }
    //printf("\n");
  }
  find_dependency(dependency_graph, has_dependency, num_cmds);
  completed[cmd_idx] = 1;
  pthread_exit(0);
}

void run_ready_commands(command_t* forrest, int** dependency_graph, int* has_dependency, int* executable, int num_cmds)
{
  //run commands with no dependency that are executable, i.e. not already running or not completed
  int i;
  int j;
  int cmd_idx;
  int all_completed = 1;
  pthread_t * threads = (pthread_t*)malloc(sizeof(pthread_t)*num_cmds);
  int * completed = (int*)malloc(sizeof(int)*num_cmds);
  while(1) {
    all_completed = 1;
    for (cmd_idx = 0; cmd_idx<num_cmds; cmd_idx++) 
      {
	if (completed[cmd_idx] == 0) 
	  {
	    all_completed = 0;
	  }	
      }
    if (all_completed == 1) 
      {
	return;
      }
    
    
    for (cmd_idx = 0; cmd_idx<num_cmds; cmd_idx++) {
      if(!has_dependency[cmd_idx] && executable[cmd_idx]) {
	executable[cmd_idx] = 0;
	struct execute_wrapper_arg* arg = (struct execute_wrapper_arg*)malloc(sizeof(struct execute_wrapper_arg));
	arg->cmd_idx = cmd_idx;
	arg->num_cmds = num_cmds;
	arg->dependency_graph = dependency_graph;
	arg->has_dependency = has_dependency;
	arg->cmd = forrest[cmd_idx];
	arg->completed = completed;
	
	pthread_create(&threads[cmd_idx], NULL, execute_wrapper, (void*)arg);
	/*pid_t pid = fork();
	  if (pid == 0) {
	  //make this command non-executable because we are about to run it
	  printf("execute command %d\n", cmd_idx);
	  executable[cmd_idx] = 0;
	  execute_command(forrest[cmd_idx]);
	  //finished executing command so we update the dependency graph to remove any dependencies on it
	  //printf("dependcency graph\n");
	  for (i = 0; i < num_cmds; i++) {
	  for (j = 0; j < num_cmds; j++) {
	  if (i == cmd_idx || j == cmd_idx) {
	  dependency_graph[i][j] = 0;
	  }
	  //printf("%d", dependency_graph[i][j]);
	  }
	  //printf("\n");
	  }
	  //now we update our dependency indicator using our updated dependency graph
	  find_dependency(dependency_graph, has_dependency, num_cmds);
	  //finally we call this function recursively to execute any executable commands who have had their dependencies resolved
	  run_ready_commands(forrest, dependency_graph, has_dependency, executable, num_cmds);
	  //while (1) {}
	  //printf("child thread exit\n");
	  return;
	  //break;//_exit(1);
	  } else {
	  continue; //parent so continue making children*/
      }
    }
  }
  
  //printf("parent function exit\n");
  //while (1);
}

void executeTimeTravel(command_t* forrest, int** dependency_graph, int num_cmds)
{
  printf("parallel command execution\n");
  int cmd_idx;
  printf("num_cmds: %d\n", num_cmds);
  int* has_dependency = (int*)malloc(sizeof(int)*num_cmds);
  int* executable = (int*)malloc(sizeof(int)*num_cmds);
  for (cmd_idx = 0; cmd_idx < num_cmds; cmd_idx++) {
    executable[cmd_idx] = 1;
    printf("execute command %d\n", cmd_idx);
    execute_command(forrest[cmd_idx]);
  }
  printf("PARRALELLIZED VERIOSN\n");
  find_dependency(dependency_graph, has_dependency, num_cmds);
  run_ready_commands(forrest, dependency_graph, has_dependency, executable, num_cmds);
}

	
