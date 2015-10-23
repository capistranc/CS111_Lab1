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

static void single_command_print(command_t cmd);

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
	  struct command* retval = list->head->child;
	  free(list->head);
	  list->head = NULL;
	  return retval;
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
  //used when we don't want to concatenate the words immediately, correct way
	return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') ||
		c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.' || c == '/' ||
		c == ':' || c == '@' || c == '^' || c == '_');
	//used initally, when I didn't realize how the word double pointer was supposed to work
	/*return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') ||
                c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.' || c == '/' ||
                c == ':' || c == '@' || c == '^' || c == '_'  ||  c == ' ');*/

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

  fprintf(stderr, "Node Pos,\tCommand Type\tWord Contained\n");
  while (currentNode != NULL)
  {
    //single_command_print((command_t)currentNode);
    token_type temp_type = currentNode->child->tok_type;
    enum command_type cmd_type = currentNode->child->type;
	  int loc = currentNode->child->pos;
	  static char const command_label[][3] = { "&&", ";", "||", "|", "S", "()" };
	  if (cmd_type == SIMPLE_COMMAND) {
	    fprintf(stderr, "%d,\t\t%d,\t\t%s",loc, temp_type, *(currentNode->child->u.word));
	    if (currentNode->child->input != NULL) {
	      //fprintf(stderr, "has input");
	      fprintf (stderr, "<%s", currentNode->child->input);
	    }
	    if (currentNode->child->output != NULL) {
	      //fprintf(stderr, "has output");
	      fprintf (stderr, ">%s", currentNode->child->output);
	    }
	    fprintf(stderr, "\n");
	  }
	  else {
	    fprintf(stderr, "%d,\t\t%d,\t\t%s\n",loc, temp_type, command_label[cmd_type]);
	    }
	  currentNode = currentNode->next;
  }
}

void word_cat(struct linked_list *list) {
  struct Node* cur_node = list->head;
  struct Node* tmp;
  token_type this_type;
  token_type next_type;
  int this_num_words;
  int next_num_words;
  int i;
  while (cur_node->next != NULL) {
    this_type = cur_node->child->tok_type;
    next_type = cur_node->next->child->tok_type;
    this_num_words = cur_node->child->num_words;
    //this command and next are words, so we concatenate them
    if (this_type == WORD && next_type == WORD) {
      char** word_buff = cur_node->child->u.word;
      char** this_word = cur_node->child->u.word;
      char** this_tmp = this_word;
      char** next_word = cur_node->next->child->u.word;
      char** next_tmp = next_word;
      //merge the sequence of char pointers in this and the next commands word** double pointer into a single double pointer array in this command
      this_num_words = cur_node->child->num_words;
      next_num_words = cur_node->next->child->num_words;
      word_buff = (char **)malloc(sizeof(char*)*(this_num_words + next_num_words + 1));
      for (i = 0; i < this_num_words; i++) {
	word_buff[i] = this_word[i];
	//fprintf(stderr, "%d:%s ", i, *(word_buff + i));
      }
      for (i = 0; i < next_num_words; i++) {
	word_buff[i + this_num_words] = next_word[i];
	cur_node->child->num_words++;
	//fprintf(stderr, "%d:%s ", i + this_num_words, *(word_buff + i + this_num_words));
      }
      word_buff[this_num_words + next_num_words] = NULL;
      //fprintf(stderr, "\n");
      //fprintf(stderr, "%d\n", cur_node->child->num_words);
      //free this and the next word
      free(next_tmp);
      free(this_tmp);
      //set this word buffer to the sequence of char pointers we created
      cur_node->child->u.word = word_buff;
      //print_command(cur_node->child);
      //remove the next node as we have extracted the useful relevant information
      tmp = cur_node->next;
      cur_node->next->next->prev = cur_node;
      cur_node->next = cur_node->next->next;
      free(tmp);
    }
    //not a concatenation so traverse to next node
    else {
      cur_node = cur_node->next;
    }
  }
}
      

void io_redirect(struct linked_list *list) {
  struct Node* cur_node = list->head;
  struct Node* tmp;
  int next_num_words;
  enum command_type this_type;
  token_type next_tok_type;
  while (cur_node->next != NULL) {
    this_type = cur_node->child->type;
    if (this_type == SIMPLE_COMMAND || this_type == SUBSHELL_COMMAND) {
      next_tok_type = cur_node->next->child->tok_type;
      next_num_words = cur_node->next->child->num_words;
      if (next_tok_type == LEFT_ARROW || next_tok_type == RIGHT_ARROW) {
	//store the token after the next's word in the appropriate io bin
	char *word = *(cur_node->next->next->child->u.word);
	if (next_num_words > 1) {
	  error(1, 0, "line:%d Bad Syntax, Node pos: %d", cur_node->child->line, cur_node->child->pos);
	}
	if (next_tok_type == LEFT_ARROW) {
	  cur_node->child->input = word;
	}
	else {
	  cur_node->child->output = word;
	}
	//remove the io operator
	tmp = cur_node->next;
	cur_node->next->next->prev = cur_node;
	cur_node->next = cur_node->next->next;
	free(tmp);
	//remove the io word
	tmp = cur_node->next;
        cur_node->next->next->prev = cur_node;
        cur_node->next = cur_node->next->next;
        free(tmp);
	//update the next token type to reflect the node deletions
	next_tok_type = cur_node->next->child->tok_type;
      }
      else {
	//not an io redirection so traverse to the next node
	cur_node = cur_node->next;
      }
    }
    else {
      cur_node = cur_node->next;
    }
  }
}

void grammarCheck(struct linked_list *list)
{
  /*
	fprintf(stderr, "This is the start of grammarCheck():\n");
	fprintf(stderr, "Node Pos,\tToken Type,\tWord\n");
  */
	struct Node* currentNode = list->head;

	int scope = 0;
	int scope_line = 0;
	int consecutive_left_arrow_count = 0;
	int consecutive_right_arrow_count = 0;
	while (currentNode != NULL)
	{
	  token_type this_tok_type;
	  token_type next_tok_type;
	  token_type prev_tok_type;
		
	  enum command_type this_type;
	  enum command_type next_type;
	  enum command_type prev_type;
		
		
	  this_tok_type = currentNode->child->tok_type;
	  this_type = currentNode->child->type;
	  if (this_tok_type == ENDTREE) {
	    currentNode = currentNode->next;
	    continue;
	  }
	  int loc = currentNode->child->pos;
	  //This simply prints the token contents - useful for debugging
	  
	  /*
	  if (this_tok_type == WORD) {
	    fprintf(stderr, "%d,\t\t%d,\t\t%s\n",loc, this_tok_type,*(currentNode->child->u.word));
	  }
	  else {
	    fprintf(stderr, "%d,\t\t%d,\t\n",loc, this_tok_type);
	    }*/
	  
	  if(currentNode->next != NULL) {
	    next_tok_type = currentNode->next->child->tok_type;
	    next_type = currentNode->next->child->type;
	  }
	  else {
	    //I don't like this method of handling null token types
	    next_tok_type = OTHER;
	  }
	  if (currentNode->prev != NULL) {
	    prev_tok_type = currentNode->prev->child->tok_type;
	    prev_type = currentNode->prev->child->type;
	  }
	  else {
	    prev_tok_type = OTHER;
	    prev_type = OTHER;
	  }
		if (this_tok_type != RIGHT_ARROW)
			consecutive_right_arrow_count = 0;
		if (this_tok_type != LEFT_ARROW)
			consecutive_left_arrow_count = 0;
	  switch (this_tok_type)
	    {
	    case WORD:
	      {
		//words are always fine
		break;
	      }
	      //here i treat newlines and semicolons the same
	      
	    case NEWLINE:
	    case SEMICOLON:
	    case PIPE:
	    case AND:
	    case OR: 
	      {
		//If two or more newlines (ENDTREE) follows an operator, this is valid
		if ( (this_tok_type == NEWLINE || this_tok_type == SEMICOLON) && next_tok_type == OTHER) {
		  //If next_tok_type == OTHER, than the next node is NULL, so we hit the end of the list.
			break;
		} 
		else {  
		  //the only thing that can follow operaters are simple commands and the beginnins of subsehsl
		  if (next_type != SIMPLE_COMMAND && next_tok_type != LEFT_PAREN) {
		    goto end_case;
		  }
		  //the only things that can precede operators are simple commands and the end of subshells
		  if (prev_type != SIMPLE_COMMAND && prev_tok_type != RIGHT_PAREN) {
		    goto end_case;
		  }
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
	    case LEFT_ARROW: {
	      consecutive_left_arrow_count++;
	      if (next_type != SIMPLE_COMMAND && next_tok_type != LEFT_PAREN) 
		{
		  if (next_tok_type == this_tok_type) {
		    consecutive_left_arrow_count++;
		  }
		  else {
		    goto end_case;
		  }
		}
	      if (prev_type != SIMPLE_COMMAND && prev_tok_type != RIGHT_PAREN) {
		if (prev_tok_type == this_tok_type) {
		  consecutive_left_arrow_count++;
		}
		else {
		  goto end_case;
		}
	      }
	      if (consecutive_left_arrow_count > 2) {
		goto end_case;
	      }
	      break;
	    }
		
		
	    case RIGHT_ARROW: {
	      consecutive_right_arrow_count++;
	      if (next_type != SIMPLE_COMMAND && next_tok_type != LEFT_PAREN) 
		  {
			if (next_tok_type == this_tok_type) {
			consecutive_right_arrow_count++;
		}
		else {
		  goto end_case;
		}
	      }
	      if (prev_type != SIMPLE_COMMAND && prev_tok_type != RIGHT_PAREN) {
		if (prev_tok_type == this_tok_type) {
		  consecutive_right_arrow_count++;
		}
		else {
		  goto end_case;
		}
	      }
	      if (consecutive_right_arrow_count > 2) {
		goto end_case;
	      }
	      break;
	    }
	    case ENDTREE: {
	      //if previous command is operator, remove this node from list
	      if (prev_type != SIMPLE_COMMAND && prev_type != SUBSHELL_COMMAND) {
		currentNode->prev->next = currentNode->next;
		currentNode->next->prev = currentNode->prev;
	      }
	      //if next command is operator throw error
	      if (next_type != SIMPLE_COMMAND && next_type != SUBSHELL_COMMAND && next_tok_type != OTHER) {
		goto end_case;
	      }
	      //this endtree token is valid, so we now check for unbalanced scope
	      if (scope != 0) {
		error(1, 0, ":%d Bad Syntax caused by unbalanced scope", scope_line);
	      }
	      break;
	    }
	    default: {
	      end_case:
	      fprintf(stderr, "\n\nGrammarCheck Failed: Variable values:\n this_tok_type: %d\t, next_tok_type: %d\n, Scope:%d\t", this_tok_type, next_tok_type, scope);
	      error(1, 0, "line:%d Bad Syntax, Node pos: %d", currentNode->child->line, currentNode->child->pos);
	      //goto end_Ccase
	      break;
	    }
	      
	    }
	  
	  currentNode = currentNode->next; // traverse to next node
	}

	//we also check for unbalanced scope after exhausting the token list
	if (scope != 0) {
	  error(1, 0, ":%d Bad Syntax caused by scope", scope_line);
	}
	
	//fprintf(stderr, "\nA successful grammarCheck has run.\n");
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
		temp->input = NULL;
		temp->output = NULL;
		temp->num_words = 0;
		current = buffer[iter];
		next = buffer[iter + 1];

		switch (current)
		{
			//Skip blank spaces and new lines
		case '\t':
		case ' ': 
		{
		  //sets the blank command to a word for later parsing
		  temp->tok_type = WORD;
		  temp->tok_type = SIMPLE_COMMAND;
		  //iterates over contiguous whitespace in the buffer
		  while (buffer[iter] == ' ' || buffer [iter] == '\t') {
		    iter++;
		  }
		  continue;
		}

		case ';': 
		{
			temp->tok_type = SEMICOLON;
			temp->type = SEQUENCE_COMMAND;
			break;
		}
		
		case '|':
		{
			if (current == next)
			{
				temp->tok_type = OR;
				temp->type = OR_COMMAND;
				iter++;
				while (buffer[iter+1] == '\n')
					iter++;
				//fprintf(stderr, "WE are in OReo CITY %d\n", iter);
			}
			else
			{
				temp->tok_type = PIPE;
				temp->type = PIPE_COMMAND;
				while (buffer[iter+1] == '\n')
					iter++;
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
				while (buffer[iter+1] == '\n')
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
		
		case '\n':
		{
			if (current == next)
			{
				iter++;
				line_num++;
				while (buffer[iter+1] == '\n')
				{
					line_num++;
					iter++;
				}
				temp->tok_type = ENDTREE;
				temp->type = SEQUENCE_COMMAND;
			}
			else {
				temp->tok_type = NEWLINE;
				temp->type = SEQUENCE_COMMAND;
				line_num++;
			}
			break;
		}
		case '(':
		{
			temp->tok_type = LEFT_PAREN;
			temp->type = SUBSHELL_COMMAND;
			break;
		}
		case ')':
		{
		  temp->tok_type = RIGHT_PAREN;
		  temp->type = SUBSHELL_COMMAND;
		  break;
		}
		case '<':
		{
			temp->tok_type = LEFT_ARROW;
			temp->type = SIMPLE_COMMAND;
			temp->u.word = (char**)malloc(sizeof(char*));
			*(temp->u.word) = "<";
			break;
		}
		case '>':
		{
			temp->tok_type = RIGHT_ARROW;
			temp->type = SIMPLE_COMMAND;
			temp->u.word = (char**)malloc(sizeof(char*));
			*(temp->u.word) = ">";
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
			
			if (minibuf[len - 1] == ' ')
				minibuf[len - 1] = '\0';
			else
				minibuf[len] = '\0';
			
			temp->num_words = 1;

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

	//printf(stderr, "%s", buffer);
	return buffer;
}

static void single_command_print(command_t c) {

  fprintf(stderr, "EXAMINING:  \n");

  token_type temp_type = c->tok_type;
  enum command_type cmd_type = c->type;
  int loc = c->pos;
  static char const command_label[][3] = { "&&", ";", "||", "|" };
  if (cmd_type == SIMPLE_COMMAND) {
    fprintf(stderr, "%d,\t\t%d,\t\t%s",loc, temp_type, *(c->u.word));
    if (c->input != NULL) {
      fprintf(stderr, "has input");
      fprintf (stderr, "<%s", c->input);
    }
    if (c->output != NULL) {
      fprintf(stderr, "has output");
      fprintf (stderr, ">%s", c->output);
    }
    fprintf(stderr, "\n");
  }
  else {
    fprintf(stderr, "%d,\t\t%d,\t\t%s\n",loc, temp_type, command_label[cmd_type]);
  }
}

int precedence(struct command* cmd1) {
  enum command_type cmd_type = cmd1->type;
  switch (cmd_type) 
    {
    case PIPE_COMMAND: {
      return 1;
      break;
    }
    case OR_COMMAND:
    case AND_COMMAND: {
      return 2;
      break;
    }
    case SEQUENCE_COMMAND: {
      return 3;
      break;
    }
    default: {
      return 10000;
      break;
    }
    }
}

command_stream_t
make_command_stream(int(*get_next_byte) (void *),
	void *get_next_byte_argument)
{
	//fprintf(stderr, "make command stream begin\n");
	char *buffer;
	buffer = create_buffer(get_next_byte, get_next_byte_argument);
	
	
	struct linked_list *tok_list = create_token_list(buffer); //need to define buffer
	grammarCheck(tok_list);
	//printTokenList(tok_list);
	word_cat(tok_list);
	//printTokenList(tok_list);
	io_redirect(tok_list);
<<<<<<< HEAD
        //printTokenList(tok_list);
		
=======
    printTokenList(tok_list);
	
>>>>>>> 7623d04362e0d0fbc528826bc3fb3cedd6acd864
	//fprintf(stderr, "command stream construction begin\n");
	
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

	//must pop newline off end of token list
	struct command* end_token;
	end_token = RemoveAtTail(tok_list);
	if(end_token->type != SEQUENCE_COMMAND) {
	  InsertAtTail(end_token, tok_list); 
	}

	while (!empty(tok_list)) 
	{
		while ((next_token = RemoveAtHead(tok_list)) != NULL) {
			if (next_token->tok_type == ENDTREE) {
			  //fprintf(stderr, "New tree\n");
			  break;
			}
			//If c is an operator or subshell command, push to command stack
			if (next_token->type != SIMPLE_COMMAND) {
			  if (next_token->type == SUBSHELL_COMMAND) {
			    if (next_token->tok_type == LEFT_PAREN)
			      {
				InsertAtHead(next_token, op_stack);
			      }
			    else if(next_token->tok_type == RIGHT_PAREN)
			      {
				op = RemoveAtHead(op_stack);
				while (op != NULL && op->type != SUBSHELL_COMMAND) {
				  //algorithm works the same way as before with the operators
				  single_command_print(op);
				  cmd2 = RemoveAtHead(cmd_stack);
				  cmd1 = RemoveAtHead(cmd_stack);
				  //new_cmd = combine(cmd1, cmd2, op);
				  op->u.command[0] = cmd1;
				  op->u.command[1] = cmd2;
				  InsertAtHead(op, cmd_stack);
				  op = RemoveAtHead(op_stack);
				}
				//single_command_print(op);
				cmd1 = RemoveAtHead(cmd_stack);
				next_token->u.subshell_command = cmd1;
				InsertAtHead(next_token, cmd_stack);
			      }
			  }
				//If operator stack is empty
			  else if (empty(op_stack)) {
			    //append c to operator stack
			    InsertAtHead(next_token, op_stack);
			  }
			  else {
					//while the precedence of the top_operater is greater than than
					//the precedence of c, pop the operator and command stacks to 
					//evaulate both/build thier trees in the correct order
					top_op = peek(op_stack);
					//can compare enumerated types beacuse converted to ints in assembly
					while ((top_op != NULL) && (precedence(next_token) >= precedence(top_op))) {
						op = RemoveAtHead(op_stack);
						cmd2 = RemoveAtHead(cmd_stack);
						cmd1 = RemoveAtHead(cmd_stack);
						//for this lab combine means setting the children pointers
						//of the operator to the two commands and pushing the
						//operator onto the command stack
						//fprintf(stderr, 'Combining commands and operator\n');
						if (cmd1 == NULL || cmd2 == NULL) {
						  fprintf(stderr, "Pushing null command onto stack - oops...");
						}
						static char const command_label[][3] = { "&&", ";", "||", "|" };
						//fprintf(stderr, "\\\n%*s%s\n", 1, "", command_label[cmd1->type]);
						//fprintf(stderr, "\\\n%*s%s\n", 1, "", command_label[cmd2->type]);
						op->u.command[0] = cmd1;
						op->u.command[1] = cmd2;
						//fprintf(stderr, "\\\n%*s%s\n", 1, "", command_label[op->u.command[0]->type]);
						//fprintf(stderr, "\\\n%*s%s\n", 1, "", command_label[op->u.command[1]->type]);
						//here i operate on the highest precedent command first
						//although in my notes i operate weith c instdead (idk why)
						//fprintf(stderr, 'Pushing new operator onto command stack\n');
						InsertAtHead(op, cmd_stack);
						top_op = peek(op_stack);
						if (top_op == NULL)
							break;
					}
					//c is now the highest precedence operator and should be evaulate
					//first so we push onto operator stack now
					//fprintf(stderr, "Pushing next_token onto operator stack\n");
					InsertAtHead(next_token, op_stack);
				}
			}
			else {
			  //next_token is a simple command so we just add it to the command stack
			  //fprintf(stderr, "Pushing next token onto command stack\n");
			  InsertAtHead(next_token, cmd_stack);
			}
		}
		//we finished reading in one complete command so we now just finish off operator and command stacks
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
		//the last command in the command stack now contatins the execution tree of the complete command
		struct command *complete_cmd_tree;
		if ((complete_cmd_tree = RemoveAtHead(cmd_stack)) == NULL) {
			continue;
		}
		InsertAtHead(complete_cmd_tree, cmd_stream->forrest);
	}
       
	//error (1, 0, "command reading not yet implemented");
	//return 0;
	//fprintf(stderr, "command tree complete\n");
	return (command_stream_t)cmd_stream;
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	//error (1, 0, "command reading not yet implemented");
	//return 0;
  
	command_t cmd;
	if ((cmd = RemoveAtTail(s->forrest)) == NULL) {
	  return 0;
	}
	return cmd;
}
