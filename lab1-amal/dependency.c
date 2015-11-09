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
  //printf("appendIO starting\n");
	unsigned int numReads = node->readCount;
	unsigned int numWrites = node->writeCount;
	unsigned int readCap = node->readBufferCap;
	unsigned int writeCap = node->writeBufferCap;

	if (numReads == node->readBufferCap)
	{
		readCap += readCap;
		node->readFiles = (char **)checked_realloc(node->readFiles, sizeof(char*) * readCap);
	}
	if (numWrites == writeCap)
	{
		writeCap += writeCap;
		node->writeFiles = (char **)checked_realloc(node->writeFiles, sizeof(char*) * writeCap);
	}


	if (command->input != NULL)
		node->readFiles[numReads++] = command->input;
	if (command->output != NULL)
		node->writeFiles[numWrites++] = command->output;

	if (command->type == SIMPLE_COMMAND)
	{

		char** words = command->u.word;
		// start at word[1] since word[0] is the name of the command
		unsigned int i;
		for (i = 1; words[i] != NULL; i++)
		{
			// if the first char of the string is not a -
			if (numReads >= readCap - 1)
			{
				readCap += readCap;
				node->readFiles = (char **)checked_realloc(node->readFiles, sizeof(char*) * readCap);
			}
			if (words[i][0] != '-')
				node->readFiles[numReads++] = words[i];
		}

	}
	node->readCount = numReads;
	node->writeCount = numWrites;

}

//Recursive function for creating the file list from the given command tree
void setRWfiles(node_t node, command_t command) {
  //printf("setRWfiles starting\n");
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


dependency_graph_t buildGraph(command_stream_t command_stream, command_t* forrest) {

  //printf("buildGraph starting\n");
	dependency_graph_t graph = makeNewGraph();

	int count = 0;
	command_t command = NULL;
	unsigned int i;
	while ((command = read_command_stream(command_stream)))
	{
	  forrest[count] = command;
		node_t temp = makeNewNode();
		temp->nid = count;
		setRWfiles(temp, command);
		/*
		//printf("node[%d], read: %d write: %d \n", count, temp->readCount, temp->writeCount);
		//printf("readFiles: ");
	       for (i = 0; i < temp->readCount; i++)
			printf("%s ", temp->readFiles[i]);
		//printf("\nwriteFiles: ");
		//for (i = 0; i < temp->writeCount; i++)
		//	printf("%s ", temp->writeFiles[i]);
		*/
		if ((unsigned int)count == graph->capacity)
			graph->capacity *= 1.5;
			graph->nodeArray = (node_t *)checked_realloc(graph->nodeArray, sizeof(node_t) * graph->capacity);
		
		graph->nodeArray[count++] = temp;
	}	

	graph->len = count;
	return graph;
}

//Helper functons for connectGraph
void printDependenceRow(node_t node, unsigned int length) {
	printf("node[%d] : ", node->nid);
	unsigned int i = 0;
	for (; i < length; i++)
		printf("%d ", node->dependence_row[i]);
	printf("\n");
}

void findRAW(node_t first, node_t dependent) {
	
	if (dependent->dependence_row[first->nid] == 1)
		return;
	
	unsigned int i,j;
	for (i = 0; i < first->writeCount; i++)
	{
		if (first->writeFiles[i] == NULL)
				break;

		for (j = 0; j < dependent->readCount; j++) 
		{
			if (dependent->readFiles[j] == NULL)
				break;
			
//			printf("%s", first->writeFiles[i]);
			if (strcmp(first->writeFiles[i], dependent->readFiles[j]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				return;
			}
		}
	}
	
}

void findWAR(node_t first, node_t dependent) {

	if (dependent->dependence_row[first->nid] == 1)
		return;
	
	
	unsigned int i,j;
	
	if (first->nid < 0)
		error(1,0, "bad nid");
	for (i = 0; i < dependent->writeCount; i++)
	{
		if (dependent->writeFiles[i] == NULL)
				break;

		for (j = 0; j < first->readCount; j++) 
		{

			if (first->readFiles[j] == NULL)
				break;	

			if (strcmp(first->readFiles[j], dependent->writeFiles[i]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				return;
			}
		}
	}

}

void findWAW(node_t first, node_t dependent) {
		
	unsigned int i = 0;
	unsigned int j = 0;

	for (i = 0;i < first->writeCount; i++)
	{
		if (first->writeFiles[i] == NULL)
			break;

		for (j = 0; j < dependent->writeCount; j++)
		  {
		
			if (dependent->writeFiles[j] == NULL)
				break;
			unsigned int k = 0;
		
		  

			if (strcmp(first->writeFiles[i] , dependent->writeFiles[j]) == 0)
			{
				dependent->dependence_row[first->nid] = 1;
				//printf("depRow : %d", dependent->dependence_row[first->nid]);
				return;
			}
		  }
	}

}

void connectGraph(dependency_graph_t graph) {
  //printf("connectGraph starting\n");
	unsigned int i,j;
	
	unsigned int length = graph->len;
	//printf("len: %d\n", length);
	
	for (i = 0; i < length - 1; i++)
	{
		for (j = i + 1; j < length; j++)
		{
			//printf("node[%d][%d] WAW run 1\n", i, j);
			findWAW(graph->nodeArray[i], graph->nodeArray[j]);
			//printf(" RAW 2\n");
			findRAW(graph->nodeArray[i], graph->nodeArray[j]);
			//printf(" WAR 3\n");
			findWAR(graph->nodeArray[i], graph->nodeArray[j]);
		}
	//printf("node[%d][%d] ith done \n", i, j);
	}
	//printf("done!\n");
	
	//printf("dependencyRowPrint\n");
	//for (i = 0; i < length; i++)
	 // printDependenceRow(graph->nodeArray[i], length);
	
	//printf("depPrintDone!\n");

	
	return;

}

//Helper functions for making matrix
int **createMatrix(dependency_graph_t graph) 
{
	int length = graph->len;
	int **matrix = (int **)checked_malloc((length+1) * sizeof(int *));
	int i;
	for (i = 0; i < length; i++) {
	  matrix[i] = graph->nodeArray[i]->dependence_row;
	}

	return matrix;
}

void printMatrix(int ** m, int len) {
	int i = 0;
	int j = 0;

	for (;i < len; i++) 
	{
		for (j = 0;j < len; j++)
		{
			printf("%d ", m[i][j]);
		}
		printf("\n");
	}
}

//Main Function
int** create_dependecy_graph(command_stream_t command_stream, command_t* forrest) {

  dependency_graph_t graph = buildGraph(command_stream, forrest);
	connectGraph(graph);
	int **matrix = createMatrix(graph); //create
	
	//Test matrix for printer
	/*
	int i, j;
	int **test = (int **)checked_malloc(sizeof(int *) * 6);
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 5; j++) 
		{
			test[i][j] = i + j;
		}
	}
	
	printMatrix(test, 5);
	*/

	printMatrix(matrix, graph->len);
	return matrix;
}
