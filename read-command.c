// UCLA CS 111 Lab 1 command reading

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>


////////////////Structure Definitions
/////////////////////////////////////////////

struct linked_list
{
	struct Node* head;
	//void (*InsertAtHead) (struct command *cmd);
	//void (*InsertAtTail) (struct command *cmd);
	//void (*RemoveAtHead) ();
	//void (*RemoveAtTail) ();
	//int (*length) ();
	//int (*empty) ();
	
};

struct Node
{
	struct command *child;
	struct Node *next;
	struct Node *prev;
};




struct command_stream
{
	//linked list of trees
	struct linked_list *forrest;
};

////////////////Helper Functions
/////////////////////////////////////////////


struct linked_list* get_new_list() 
{
	struct linked_list* new_list = (struct linked_list*)malloc(sizeof(struct linked_list));
	new_list->head = NULL;
	return new_list;
}


//Creates a new Node and returns pointer to it. 
struct Node* GetNewNode(struct command *cmd) {
	struct Node* newNode
		= (struct Node*)malloc(sizeof(struct Node));
	newNode->child = cmd;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

//Inserts a Node at head of doubly linked list
void InsertAtHead(struct command *cmd, struct linked_list *list) {
	struct Node* newNode = GetNewNode(cmd);
	if (list->head == NULL) {
		list->head = newNode;
		return;
	}
	list->head->prev = newNode;
	newNode->next = list->head;
	list->head = newNode;
}

struct command* RemoveAtHead(struct linked_list *list)
{
	if (list->head == NULL) {
		return NULL;
	}
	struct command* retval;
	if (list->head->next == NULL) {
	  retval = list->head->child;
	  free(list->head);
	  list->head = NULL;
	  return retval;
	}
	retval = list->head->child;
	list->head = list->head->next;
	free(list->head->prev);
	list->head->prev = NULL;
	return retval;
}

//Inserts a Node at tail of Doubly linked list
void InsertAtTail(struct command *cmd, struct linked_list *list) {
	struct Node* temp = list->head;
	struct Node* newNode = GetNewNode(cmd);
	if (list->head == NULL) {
		list->head = newNode;
		return;
	}
	while (temp->next != NULL) temp = temp->next; // Go To last Node
	temp->next = newNode;
	newNode->prev = temp;
}

struct command* RemoveAtTail(struct linked_list *list) {
	if (list->head == NULL) {
		return NULL;
	}
	if (list->head->next == NULL) {
		free(list->head);
		list->head = NULL;
		return list->head->child;
	}
	struct Node* temp = list->head;
	while (temp->next != NULL) temp = temp->next; // Go to last node
	struct command* retval = temp->child;
	temp = temp->prev;
	free(temp->next);
	temp->next = NULL;
	return retval;
}

struct command* peek(struct linked_list *list) {
	if (list->head == NULL) {
		return NULL;
	}
	return list->head->child;
}

int length(struct linked_list *list)
{
	int count = 0;
	struct Node* temp = list->head;
	while (temp != NULL)
	{
		temp = temp->next;
		count++;
	}
	return count;
}

int empty(struct linked_list *list)
{
	if (list->head == NULL)
	{
		return 1;
	}
	return 0;
}

//USED FOR TESTING LINKED LIST
/*
//Prints all the elements in linked list in forward traversal order
void Print(struct linked_list *list) {
struct Node* temp = list->head;
printf("Forward: ");
while(temp != NULL) {
printf("%d ",temp->data);
temp = temp->next;
}
printf("\n");
}*/

bool isValidChar(char c)
{
	return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') ||
		c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.' || c == '/' ||
		c == ':' || c == '@' || c == '^' || c == '_');
}
//////////Primary Functions//////////////////
////////////////////////////////////////////

int valid_operator(char c, char* word, enum command_type *cmd, struct linked_list *cmd_stack,
	int(*get_next_byte) (void *), void *get_next_byte_argument)
{
	char next;
	//char *tmp;
	int retval = 0;
	if (c == ';') {
		*cmd = SEQUENCE_COMMAND;
		retval = 1;
	}
	else if (c == '(' || c == ')') {
		*cmd = SUBSHELL_COMMAND;
		retval = 1;
	}
	else if (c == '|') {
		next = (char)get_next_byte(get_next_byte_argument);
		if (next == '|') {
			*cmd = OR_COMMAND;
		}
		else {
			*cmd = PIPE_COMMAND;
			char *temp = (char*)malloc(sizeof(char)*(strlen(word) - 1));
		}
		retval = 1;
	}
	else if (c == '&') {
		next = get_next_byte(get_next_byte_argument);
		if (next == '&') {
			*cmd = AND_COMMAND;
		}
		retval = 1;
	}
	if (retval) {
		//gotta push nontokenized string onto the command stack
		//FIXME:  Need to clean up this nontoken string and check for invalid syntax - as of now newlines and
		//whitespace are a-ok and we gotta check for that - suggest creating another function
		struct command simple_cmd;
		simple_cmd.u.word = (char**)malloc(sizeof(char*));
		*(simple_cmd.u.word) = (char*)malloc(sizeof(char)*strlen(word));
		**(simple_cmd.u.word) = *word;
		InsertAtHead(&simple_cmd, cmd_stack);
		//now clean up the reference variable used in the parser
		free(word);
		word = "";
		if (*cmd == PIPE_COMMAND) {
			//tmp = (char *)malloc(sizeof(char) * strlen(&next))
			//*tmp = next;
			strcat(word, &next);
		}
	}
	return retval;
}

struct command* combine(struct command* cmd1, struct command* cmd2, struct command* op)
{
	(op->u.command[0]) = cmd1;
	(op->u.command[1]) = cmd2;
	return op;
}

////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////CHRIS'S FUNCTIONS/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

//Purely for Debugging and checking the linked list functions correctly.
void printTokenList(struct linked_list *list)
{

  struct Node* currentNode = list->head;

  fprintf(stderr, "Node Pos, Command Type, Word Contained\n");
  while (currentNode != NULL)
  {
	  token_type temp_type = currentNode->child->tok_type;
	  int loc = currentNode->child->pos;
	  
	  if (temp_type == WORD){
		  fprintf(stderr, "%d,\t%d,\t%s\n",loc, temp_type,*(currentNode->child->u.word));
	  }
	  else
	  {
		  fprintf(stderr, "%d,\t%d,\t\n",loc, temp_type);
	  }
	  currentNode = currentNode->next;
  }
}
void grammarCheck(struct linked_list *list)
{
	struct Node* currentNode = list->head;

	int scope = 0;
	int scope_line = 0;

	while (currentNode != NULL)
	{
		token_type this_tok_type;
		token_type next_tok_type;
		token_type prev_tok_type;
		fprintf(stderr, " %d ", this_tok_type);
		enum command_type this_type;
		enum command_type next_type;
		enum command_type prev_type;
		
		
		this_tok_type = currentNode->child->tok_type;
		this_type = currentNode->child->type;
		if(currentNode->next != NULL)
		{
			next_tok_type = currentNode->next->child->tok_type;
			next_type = currentNode->next->child->type;
		}
		else
		{
			next_tok_type = -1;
			next_type = -1;
		}
		if (currentNode->prev != NULL) 
		{
			prev_tok_type = currentNode->prev->child->tok_type;
			prev_type = currentNode->prev->child->type;
		}
		else {
			prev_tok_type = -1;
			prev_type = -1;
		}
		
		switch (this_tok_type)
		{
			case WORD:
				//words are always fine
				break;
			case SEMICOLON:
			case PIPE:
			case AND:
			case OR: 
			{
				if (next_type != SIMPLE_COMMAND && next_tok_type != LEFT_PAREN) {
					goto end_case;
				}
				if (prev_type != SIMPLE_COMMAND && prev_tok_type != RIGHT_PAREN) {
					goto end_case;
				}
				break;
			}
			case LEFT_PAREN: {
				scope++;
				if (scope_line == 0) {
					scope_line = currentNode->child->line;
				}
				break;
			}
			case RIGHT_PAREN: {
				scope--;
				if (scope < 0) {
					goto end_case;
				}
				if (scope == 0) {
					scope_line = 0;
				}
				break;
			}
			case LEFT_ARROW:
			case RIGHT_ARROW: {
				int consecutive_arrow_count = 1;
				if (next_type != SIMPLE_COMMAND && next_tok_type != LEFT_PAREN) {
					if (next_tok_type == this_tok_type) {
						consecutive_arrow_count++;
					}
					else {
						goto end_case;
					}
				}
				if (prev_type != SIMPLE_COMMAND && prev_tok_type != RIGHT_PAREN) {
					if (prev_tok_type == this_tok_type) {
						consecutive_arrow_count++;
					}
					else {
						goto end_case;
					}
				}
				if (consecutive_arrow_count > 2) {
					goto end_case;
				}
				break;
			}
			case NEWLINE: 
			{
				//if previous command is operator, remove this node from list
				if (prev_type != SIMPLE_COMMAND && prev_type != SUBSHELL_COMMAND) {
					currentNode->prev->next = currentNode->next;
					currentNode->next->prev = currentNode->prev;
				}
				//if next command is operator throw error
				if (next_type != SIMPLE_COMMAND && next_type != SUBSHELL_COMMAND) {
					goto end_case;
				}
				break;
			}
			default: 
			{
				end_case:
				fprintf(stderr, "\nThis is the tok_type: %d\n", this_tok_type);
				fprintf(stderr, "\nThis is the next_tok_type: %d\n", next_tok_type);
				error(1, 0, ":%d Bad Syntax, Node pos: %d", currentNode->child->line, currentNode->child->pos);
		
				//goto end_Ccase
				break;
			}
		}
		
		currentNode = currentNode->next; // Go To last Node
	}

	if (scope != 0) {
			error(1, 0, ":%d Bad Syntax caused by scope", scope_line);
	}
	
	fprintf(stderr, "\n A successful grammarCheck has run.\n");
}



struct linked_list* create_token_list(char* buffer)
{
	struct linked_list *tok_list = get_new_list(); //
	int iter = 0;
	int line_num = 1;
	int node_pos = 0;

	char current;
	char next;

	while ((buffer[iter] != '\0') && (buffer[iter] != EOF))
	{
		command_t temp = (command_t)checked_malloc(sizeof(command_t));
		current = buffer[iter];
		next = buffer[iter + 1];

		switch (current)
		{
			//Skip blank spaces and new lines
		case '\t':
		case ' ': 
		{
			iter++;
			continue;
		}
		
		case ';': 
		{
			temp->tok_type = SEMICOLON;
			temp->type = SEQUENCE_COMMAND;
			break;
		}
		
		case '\n':
		{
			if (current == next)
			{
				temp->tok_type = -1;
				temp->type = -1;
			}
			else {
				temp->tok_type = NEWLINE;
				temp->type = SEQUENCE_COMMAND;
				line_num++;
			}
			break;
		}
		case '|':
		{
			if (current == next)
			{
				temp->tok_type = OR;
				temp->type = OR_COMMAND;
				iter++;
				//fprintf(stderr, "WE are in OReo CITY %d\n", iter);
			}
			else
			{
				temp->tok_type = PIPE;
				temp->type = PIPE_COMMAND;
				//fprintf(stderr, "Doese this PIPE ever get hit at %d\n", iter);
			}
			break;
		}
		case '&':
		{
			if (current == next)
			{
				temp->tok_type = AND;
				temp->type = AND_COMMAND;
				iter++;
				//fprintf(stderr, "Does this statement ever get hit at %d\n", iter);
				break;
			}
			else
			{
				temp->tok_type = OTHER;
				//fprintf(stderr, "Does this OTHER statement get hit at %d\n", iter);
			}
			break;
		}
		case '(':
		{
			temp->tok_type = RIGHT_PAREN;
			temp->type = SUBSHELL_COMMAND;
			break;
		}
		case ')':
		{
			temp->tok_type = LEFT_PAREN;
			temp->type = SUBSHELL_COMMAND;
			break;
		}
		case '<':
		{
			temp->tok_type = LEFT_ARROW;
			temp->type = SIMPLE_COMMAND;
			break;
		}
		case '>':
		{
			temp->tok_type = RIGHT_ARROW;
			temp->type = SIMPLE_COMMAND;
			break;
		}
		default:
		{
			temp->tok_type = OTHER;
			break;
		}
		}

		int len = 1;

		//Case that the token is a word
		if (isValidChar(current))
		{
			temp->tok_type = WORD;
			temp->type = SIMPLE_COMMAND;
			//Gets the length of the word within the buffer.
			while (isValidChar(buffer[iter + len]))
				len++;

			//Allocates memory for the string stored in the token, accounting for NULL byte at end.
			temp->u.word = (char **)checked_malloc(sizeof(char*));
			
			char *minibuf = (char *)checked_malloc(sizeof(char *) * (len + 1));

			int i;
			for (i = 0; i < len; i++)
			{
				minibuf[i] = buffer[iter + i];
			}
			minibuf[len] = '\0';

			*(temp->u.word) = minibuf;
			iter += len - 1;
			//free(minibuf);  WE STIOOPL USE TJHSIO FMENRSFFN?BSA
			
		}
		else if (temp->tok_type == OTHER)
		{
			fprintf(stderr, "The following chars are a problem: current: %c, next: %c", current, next);
			error(1, 0, "Unable to tokenize string at %d", iter);
			exit(1);
		}
		
		temp->line = line_num;
		//fprintf(stderr, "test temp->line = %d\n", temp->line);

		temp->pos = node_pos++;
		InsertAtTail(temp, tok_list);

		//free(temp)  WE STILL USE THIS MEMRORNYTY
		iter++;
	}

	return tok_list;
}


//Create_buffer simply stores the file into a buffer
char* create_buffer(int(*get_next_byte) (void *), void *get_next_byte_argument)
{
	size_t iter = 0;
	size_t buffer_size = 1024;
	char *buffer = (char *)checked_malloc(buffer_size);
	char current_byte;
	

	//Iterates through entire file
	while ((current_byte = get_next_byte(get_next_byte_argument)) != EOF)
	{
		//Skips over characters included in a comment
		if (current_byte == '#')
		{
			while ((current_byte = get_next_byte(get_next_byte_argument)) != '\n')
			{
				if (current_byte == EOF)
					break;
				//fprintf(stderr, "%d", current_byte);
				//Continue iterating until the end of the comment is reached.
			}
			continue;
		}

		buffer[iter++] = current_byte;

		if (iter == buffer_size)
			buffer = (char *)checked_grow_alloc(buffer, &buffer_size);
	}
	buffer[iter] = '\0';

	fprintf(stderr, "%s", buffer);
	return buffer;
}


command_stream_t
make_command_stream(int(*get_next_byte) (void *),
	void *get_next_byte_argument)
{
	//fprintf(stderr, "make command stream begin\n");
	char *buffer;
	buffer = create_buffer(get_next_byte, get_next_byte_argument);
	
	
	struct linked_list *tok_list = create_token_list(buffer); //need to define buffer
	printTokenList(tok_list);
	//grammarCheck(tok_list);
	
	
	
	struct linked_list *op_stack = get_new_list();
	struct linked_list *cmd_stack = get_new_list();
	struct command_stream *cmd_stream = (struct command_stream*)malloc(sizeof(struct command_stream));
	cmd_stream->forrest = get_new_list();
	enum command_type *type = (enum command_type*)malloc(sizeof(enum command_type));
	struct command* op;
	struct command* top_op;
	struct command* cmd1;
	struct command* cmd2;
	struct command* next_token;
	while (!empty(tok_list)) {
		while ((next_token = RemoveAtHead(tok_list)) != NULL) {
			if (next_token->type == 1000) {
				break;
			}
			//If c is special token(i.e. operator), push to command stack
			if (next_token->type != SIMPLE_COMMAND) {
				//If operator stack is empty
				if (empty(op_stack)) {
					//append c to operator stack
					InsertAtHead(next_token, op_stack);
				}
				else {
					//while the precedence of the top_operater is greater than than
					//the precedence of c, pop the operator and command stacks to 
					//evaulate both/build thier trees in the correct order
					top_op = peek(op_stack);
					//can compare enumerated types beacuse converted to ints in assembly
					while ((top_op != NULL) && (next_token->type <= top_op->type)) {
						op = RemoveAtHead(op_stack);
						cmd2 = RemoveAtHead(cmd_stack);
						cmd1 = RemoveAtHead(cmd_stack);
						//for this lab combine means setting the children pointers
						//of the operator to the two commands and pushing the
						//operator onto the command stack
						//          new_cmd = combine(cmd1, cmd2, op);
						op->u.command[0] = cmd1;
						op->u.command[1] = cmd2;
						//here i operate on the highest precedent command first
						//although in my notes i operate weith c instdead (idk why)
						InsertAtHead(op, cmd_stack);
						top_op = peek(op_stack);
						if (top_op == NULL)
							break;
					}
					//c is now the highest precedence operator and should be evaulate
					//first so we push onto operator stack now
					InsertAtHead(next_token, op_stack);
				}
				//whever we encounter an operator i.e. tokenized character we
				//push the previous series of nontokenized characters (i.e word)
				//onto the command stack
				//cmd_stack.push(word); -- NOW HANDLED BY VALID_OPERATOR FUNCTION
			}
			else {
				//c is a non tokenized character so we just add it onto the existing
				//sequence of nontokenized characters (i.e the word)
				InsertAtHead(next_token, cmd_stack);
			}
		}
		//file stream empty so we now just finish off operator and command stacks
		//should end with empty op stack and commmand stack with answer in it
		//for our purposes would be pointer to head node of command tree
		while (!empty(op_stack)) {
			//algorithm works the same way as before with the operators
			op = RemoveAtHead(op_stack);
			cmd2 = RemoveAtHead(cmd_stack);
			cmd1 = RemoveAtHead(cmd_stack);
			//new_cmd = combine(cmd1, cmd2, op);
			op->u.command[0] = cmd1;
			op->u.command[1] = cmd2;
			InsertAtHead(op, cmd_stack);
		}
		struct command *complete_cmd_tree;
		if ((complete_cmd_tree = RemoveAtHead(cmd_stack)) == NULL) {
			continue;
		}
		InsertAtHead(complete_cmd_tree, cmd_stream->forrest);
	}
       
	error (1, 0, "command reading not yet implemented");
	//return 0;
	return (command_stream_t)cmd_stream;
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error (1, 0, "command reading not yet implemented");
	return 0;
	/*
	command_t cmd;
	if ((cmd = RemoveAtHead(s->forrest)) == NULL) {
		return 0;
	}
	return cmd; */
}
