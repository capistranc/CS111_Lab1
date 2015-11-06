// UCLA CS 111 Lab 1 command execution
#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <unistd.h>

#include <string.h> // for strcmp
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for waitpid
#include <unistd.h> // for execvp, fork
#include <stdio.h> // for fprintf
#include <stdlib.h> // for exit
#include <fcntl.h> // for file open constants


#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
static function definitions, etc.  */

typedef struct node* node_t;
typedef struct dependency_graph* dependency_graph_t;

struct node {
	int nid;

	int *dependence_row;
	unsigned int dependency_count;

	unsigned int readCount;
	unsigned int writeCount;

	char **readFiles;
	char **writeFiles;
	size_t readBufferCap;
	size_t writeBufferCap;

};

struct dependency_graph {
	node_t *nodeArray;
	size_t len;
	size_t capacity;
	int** dep_matrix; //This should be outside struct
};

//			Helper functions for the graph and nodes
//Initializes nid = -1 and allocates memory for buffers
node_t makeNewNode() {
	unsigned int cap = 16;

	node_t node = checked_malloc(sizeof(node_t));
	node->dependency_count = 0;
	node->readCount = 0;
	node->writeCount = 0;
	node->nid = -1;
	node->readBufferCap = cap;
	node->writeBufferCap = cap;
	node->dependence_row = (int *)checked_malloc(sizeof(int) * cap);
	node->readFiles = (char **)checked_malloc(sizeof(char*) * cap);
	node->writeFiles = (char **)checked_malloc(sizeof(char*) * cap);

	return node;
}

//Initializes the graph and alocates memory for the array
dependency_graph_t makeNewGraph() {
	unsigned int cap = 16;

	dependency_graph_t graph = (dependency_graph_t)checked_malloc(cap * sizeof(struct dependency_graph));

	node_t *tempArray = (node_t *)checked_malloc(sizeof(node_t) * cap);
	graph->nodeArray = tempArray;

	graph->capacity = cap;
	graph->len = 0;

	return graph;
}


//				Helper functions for buildGraph
//Populates (char**) graph->readFiles and (char **) graph->writeFiles.
void appendIO(node_t node, command_t command) {
	unsigned int numReads = node->readCount;
	unsigned int numWrites = node->writeCount;

	if (numReads == node->readBufferCap)
	{
		node->readBufferCap *= 1.5;
		node->readFiles = (char **)checked_realloc(node->readFiles, sizeof(char*) * node->readBufferCap);
	}
	if (numWrites == node->writeBufferCap)
	{
		node->writeBufferCap *= 1.5;
		node->writeFiles = (char **)checked_realloc(node->writeFiles, sizeof(char*) * node->writeBufferCap);
	}


	if (command->input != NULL)
		node->readFiles[numReads++] = command->input;
	if (command->output != NULL)
		node->writeFiles[numWrites++] = command->input;

	char** words = command->u.word;
	// start at word[1] since word[0] is the name of the command
	unsigned int i = 1;
	for (; words[i] != NULL; i++)
	{
		// if the first char of the string is not a -
		if (numReads >= node->readBufferCap)
		{
			node->readBufferCap *= 1.5;
			node->readFiles = (char **)checked_realloc(node->readFiles, sizeof(char*) * node->readBufferCap);
		}
		if (words[i][0] != '-') {
			node->readFiles[numReads++] = words[i];
		}
	}

	node->readCount = numReads;
	node->writeCount = numWrites;

}

//Recursive function for creating the file list from the given command tree
void setRWfiles(node_t node, command_t command) {

	switch (command->type)
	{
	case SIMPLE_COMMAND:
	{
		appendIO(node, command);
		break;
	}

	case SUBSHELL_COMMAND:
	{
		appendIO(node, command);
		setRWfiles(node, command->u.subshell_command);
		break;
	}
	case AND_COMMAND:
	case SEQUENCE_COMMAND:
	case OR_COMMAND:
	case PIPE_COMMAND:
		setRWfiles(node, command->u.command[0]);
		setRWfiles(node, command->u.command[1]);
		break;
	default:
		break;
	}
	return;
}


dependency_graph_t buildGraph(command_stream_t command_stream) {
	dependency_graph_t graph = makeNewGraph();

	int count = 0;
	command_t command = NULL;

	while ((command = read_command_stream(command_stream)))
	{
		node_t temp = makeNewNode();
		temp->nid = count;
		setRWfiles(temp, command);

		if ((unsigned int)count == graph->capacity)
		{
			graph->capacity *= 1.5;
			graph->nodeArray = (node_t *)checked_realloc(graph->nodeArray, sizeof(node_t) * graph->capacity);
		}
		graph->nodeArray[count++] = temp;
	}

	graph->len = count;
	return graph;
}

//Helper functons for connectGraph
void findRAW(node_t first, node_t dependent) //COMPLETE
{
	unsigned int i = 0;
	unsigned int j = 0;

	for (;i < first->writeCount; i++)
	{
		for (; j < dependent->readCount; j++)
			if (strcmp(first->writeFiles[i], dependent->readFiles[j]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				break;
			}
	}

}

void findWAR(node_t first, node_t dependent) //NOT FINISHED
{
	unsigned int i = 0;
	unsigned int j = 0;

	for (;i < first->readCount; i++)
	{
		for (; j < dependent->writeCount; j++)
			if (strcmp(first->readFiles[i], dependent->writeFiles[j]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				break;
			}
	}

}

void findWAW(node_t first, node_t dependent) //NOT FINISHED
{
	unsigned int i = 0;
	unsigned int j = 0;

	for (;i < first->writeCount; i++)
	{
		for (; j < dependent->writeCount; j++)
			if (strcmp(first->writeFiles[i], dependent->writeFiles[j]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				break;
			}
	}

}

void connectGraph(dependency_graph_t graph) {

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int length = graph->len;

	for (; i < length - 1; i++)
	{
		for (j = i + 1; j < length; j++)
		{
			findRAW(graph->nodeArray[i], graph->nodeArray[j]);
			findWAW(graph->nodeArray[i], graph->nodeArray[j]);
			findWAR(graph->nodeArray[i], graph->nodeArray[j]);
		}
	}
	return;
}

int **createMatrix(dependency_graph_t graph)
{
	int **matrix = (int **)checked_malloc(graph->len * sizeof(int *));
	unsigned int i = 0;
	for (; i < graph->len; i++)
		*matrix = graph->nodeArray[i]->dependence_row;

	return matrix;
}

int** create_dependecy_graph(command_stream_t command_stream) {

	dependency_graph_t graph = buildGraph(command_stream);
	connectGraph(graph);
	int **matrix = createMatrix(graph); //create
	error(1, 0, "Not implemented yet\n");
	return matrix;
}