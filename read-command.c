// UCLA CS 111 Lab 1 command reading

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "command.h"
#include "command-internals.h"

#include <error.h>


/* FIXME: You may need to add #include directives, macro definitions,
static function definitions, etc.  */

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

struct linked_list* get_new_list() {
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
	if (list->head->next == NULL) {
		free(list->head);
		list->head = NULL;
		return list->head->child;;
	}
	struct command* retval = list->head->child;
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


/* FIXME: Define the type 'struct command_stream' here.  This should
complete the incomplete type declaration in command.h.  */

struct command_stream
{
	//linked list of trees
	struct linked_list *forrest;
};

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
			word = strcat(word, &next);
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


command_stream_t
make_command_stream(int(*get_next_byte) (void *),
	void *get_next_byte_argument)
{
	int done = 0;
	char* word;
	char c = (char)get_next_byte(get_next_byte_argument);
	struct linked_list *op_stack = get_new_list();
	struct linked_list *cmd_stack = get_new_list();
	struct command_stream *cmd_stream = (struct command_stream*)malloc(sizeof(struct command_stream));
	cmd_stream->forrest = get_new_list();
	enum command_type *type = (enum command_type*)malloc(sizeof(enum command_type));
	struct command* op;
	struct command* top_op;
	struct command* cmd1;
	struct command* cmd2;
	while (!done) {
		while (!feof(get_next_byte_argument)) {
			//If c is special token(i.e. operator), push to command stack
			if (valid_operator(c, word, type, cmd_stack, get_next_byte, get_next_byte_argument)) {
				struct command new_op;
				new_op.type = *type;
				//If operator stack is empty
				if (empty(op_stack)) {
					//append c to operator stack
					InsertAtHead(&new_op, op_stack);
				}
				else {
					//while the precedence of the top_operater is greater than than
					//the precedence of c, pop the operator and command stacks to 
					//evaulate both/build thier trees in the correct order
					top_op = peek(op_stack);
					//can compare enumerated types beacuse converted to ints in assembly
					while ((top_op != NULL) && (new_op.type <= top_op->type)) {
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
					InsertAtHead(&new_op, op_stack);
				}
				//whever we encounter an operator i.e. tokenized character we
				//push the previous series of nontokenized characters (i.e word)
				//onto the command stack
				//cmd_stack.push(word); -- NOW HANDLED BY VALID_OPERATOR FUNCTION
			}
			else {
				//c is a non tokenized character so we just add it onto the existing
				//sequence of nontokenized characters (i.e the word)
				//char* tmp;
				//tmp = (char*)malloc(sizeof(char) * strlen(c))
				//*tmp = c;  
				word = strcat(word, &c);
				if (c == '\n' && word[strlen(word) - 1] == '\n') {
					break;
				}
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
		struct command *final_cmd_tree = RemoveAtHead(cmd_stack);
		InsertAtHead(final_cmd_tree, cmd_stream->forrest);
	}

	return (command_stream_t)cmd_stream;
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	//error (1, 0, "command reading not yet implemented");
	//return 0;
	return RemoveAtHead(s->forrest);
}
