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

int
command_status (command_t c)
{
	return c->status;
}

void handle_io(command_t c)
{
	if (c->input != NULL) {
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

typedef struct dependency_node* dependency_node_t;
typedef struct dependency_graph* dependency_graph_t;

struct dependency_node{
	command_t cmd;
	unsigned int nid;
	
	unsigned int dependency_count;
	
	unsigned int readCount;
	unsigned int writeCount;
	char **readFiles;
	char **writeFiles;
	
	dependency_node_t *dependency_list;
};



struct dependency_graph{
	dependency_node_t *list;
	unsigned int len;
	size_t capacity;
	int** dep_matrix;
};




void addNodetoGraph(dependency_graph_t graph, dependency_node_t node) {

	if (graph->list == NULL || graph->len == 0) {
		graph->capacity = 16;
		graph->list = (dependency_node_t *)checked_malloc(graph->capacity * sizeof(struct dependency_node*));
		node->nid = graph->len;
		graph->list[graph->len++] = node;
	}
	else
	{
		if (graph->len == graph->capacity - 1)
		{
			graph->capacity += graph->capacity;
			graph->list = (dependency_node_t *)checked_realloc(graph->list, graph->capacity * sizeof(struct dependency_node*));
		}

		node->nid = graph->len;
		graph->list[graph->len++] = node;
	}

}

void setRWfiles(command_t command, dependency_graph_t list)
{
	size_t buffercapacity = 1;
	
	char **readArray = (char **) checked_malloc(sizeof(char*) * buffercapacity);
	char **writeArray = (char **) checked_malloc(sizeof(char*) * buffercapacity);
	//dependency_graph_t dep_tree = (dependency_graph_t)checked_malloc(sizeof(struct dependence graph));
	//dependency_node_t list = (dependency_node_t)checked_malloc(capacity * sizeof(struct dependency_graph);



	switch (command->type)
	{
		case SIMPLE_COMMAND:
		{
			buffercapacity = 4;
			dependency_node_t temp = checked_malloc(sizeof(struct dependency_node));
			unsigned int readIndex = 0;
			unsigned int writeIndex = 0;
			
			if (command->input != NULL)
			{	
				readArray[readIndex++] = command->input;
			}
			if (command->output != NULL)
			{
				writeArray[writeIndex++] = command->output;
			}
				//&(dep_node->readFiles);
				
			char** words = command->u.word;
			// start at word[1] since word[0] is the name of the command
			unsigned int i = 1;
			for (; words[i] != NULL; i++)
			{	
				// if the first char of the string is not a -
				if (i >= buffercapacity)
				{
					buffercapacity *= 2;
					readArray = (char **)checked_realloc(readArray, sizeof(char*) * buffercapacity);
				}
				if(words[i][0] != '-') {
					readArray[readIndex++] = words[i];
				}
			}
			
			//Test this works//

			temp->writeCount = writeIndex;
			temp->readCount = readIndex;
			temp->readFiles = readArray;
			temp->writeFiles = writeArray;

			addNodetoGraph(list, temp);
			break;
		}
				
		case SUBSHELL_COMMAND:
		{
			int readIndex = 0;
			int writeIndex = 0;

			dependency_node_t temp = checked_malloc(sizeof(struct dependency_node));
			if (command->input != NULL)
			{
				readArray[readIndex++] = command->input;
			}
			if (command->output != NULL)
			{
				writeArray[writeIndex++] = command->output;
			}

			temp->writeCount = writeIndex;
			temp->readCount = readIndex;
			temp->readFiles = readArray;
			temp->writeFiles = writeArray;

			addNodetoGraph(list, temp);
			setRWfiles(command->u.subshell_command, list);
			
			break;
		}
		case AND_COMMAND:
		case SEQUENCE_COMMAND:
		case OR_COMMAND:
		case PIPE_COMMAND:
			setRWfiles(command->u.command[0], list);
			setRWfiles(command->u.command[1], list);
			break;
		default:
			break;
	}
	return;
}

dependency_graph_t buildGraph(command_stream_t command_stream)
{
	
	command_t command = NULL;
	unsigned int count = 0;
	int capacity = 16;
	//dependency_node_t dep_node = checked_malloc(sizeof(dependency_node_t));
	
	//dependency_node_t list = checked_malloc(capacity * sizeof(struct dependency_node));
	dependency_graph_t tree = (dependency_graph_t)checked_malloc(capacity * sizeof(struct dependency_graph));
	tree->len = 0;
	tree->list = NULL;

	while ((command = read_command_stream (command_stream)))
	{
		setRWfiles(command, tree);
	}

	unsigned int i = 0;
	unsigned int j = 0;
	printf("Graph Size: %d \n", tree->len);
	for (;i < tree->len; i++)
	{
		for (;j < tree->list[i]->readCount;j++)
			printf("nodeRead[%d]: %s\n", i, tree->list[i]->readFiles[j]);
		for (;j < tree->list[i]->writeCount; j++)
			printf("nodeWrite[%d]: %s\n", i, tree->list[i]->writeFiles[j]);
	}
	int **matrix = (int **) checked_malloc(sizeof(int *) * tree->len);
	tree->dep_matrix = matrix;
	return tree;
}

void createEdges(dependency_graph_t graph)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int l = 0;

	printf("NumNodes: %d\n", graph->len);
	
	for (;i < graph->len - 1; i++) 
	{
		for (j = i+1 ;j < graph->len; j++) //Compare one node with another
		{
			unsigned int warDependencies = 0;
			for (k = 0; k < graph->list[i]->readCount; k++) //Checks for WAR
			{
				for (l = 0; l < graph->list[j]->writeCount; l++)
				{
					if (graph->list[i]->readFiles[k] == graph->list[j]->writeFiles[l])
					{
						warDependencies++;
						graph->dep_matrix[i][j] += 1;//node i depends on j;
					}
				}	
			}
			graph->list[i]->dependency_count += warDependencies;
			
			
			unsigned int wawDependencies = 0;
			unsigned int rawDependencies = 0;
			for (k = 0; k < graph->list[i]->writeCount; k++) //
			{
				for (l = 0; l < graph->list[j]->writeCount; l++) //This checks for WAW
				{
					if (graph->list[i]->writeFiles[k] == graph->list[j]->writeFiles[l])
					{
						wawDependencies++;
						graph->dep_matrix[j][i] += 1; //node j depends on node i;
					}
				}
				
				for (l = 0; l < graph->list[j]->readCount; l++) //This checks for RAW
				{
					if (graph->list[i]->writeFiles[k] == graph->list[j]->readFiles[l])
					{
						rawDependencies++;
						graph->dep_matrix[j][i] += 1; //node j depends on i
					}
				}
				
			}
			graph->list[j]->dependency_count += rawDependencies + wawDependencies;
		}
	}
	
}

void printMatrix(int** m, int len)
{
	int i = 0;
	int j = 0;
	for (; i < len; i++) 
	{
		for (; j < len; j++) 
		{
			printf("%d", m[i][j]);
		}
		printf("\n");
	}
}

void executeTimeTravel(command_stream_t command_stream)
{
	int count = 0;
	command_t command;
	
	//int capacity = 32;
	//dependency_node_t list = calloc(capacity, sizeof(struct dependency_node));
	//dependency_graph_t graph = checked_malloc(sizeof(struct dependency_graph));
	
	dependency_graph_t graph = buildGraph(command_stream);
	createEdges(graph);
	//printMatrix(graph->dep_matrix, graph->len);
	printf("matrix: \n%d ", graph->dep_matrix[0][0]);
	
	error(1,0,"Not implemented yet\n");
}

	
//dependency_graph build_dependency_graph()