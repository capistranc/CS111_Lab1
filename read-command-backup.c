// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>

//Structure Definitions

struct command_stream {
	command_t command;
	command_stream_t next;
	command_stream_t prev;
};

struct token {
	token_type type;
	char* word;
};

struct token_list {
	token t;
	token_list_t next;
	token_list_t prev;
};

///////////////   

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */

bool isValidChar(char c)
{
	return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9'));
}

bool isValidOp(char c)
{
	return (c == '|' || c == '&' || c == '(' || c == ')' || c == '<' || c == '>' );

	/*(c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.' || c == '/' ||
	c == ':' ||	c == '@' || c == '^' || c == '-' ||
	c == ';' || c == '|' || c == '&' || c == '(' || c == ')' || c == '<' || c == '>' ||
	c == '\n' || c == '\t' || c == ' ' || c == '#')	; */
}


token_list_t create_token_list(char* buffer)
{
	token_list_t head = NULL;
	head = (token_list_t)checked_malloc(sizeof(token_list_t));
	head->next = NULL;
	head->prev = NULL;
	

	token_list_t traverse = (token_list_t)checked_malloc(sizeof(token_list_t));
	
	token_type type;

	int iter = 0;

	char current;
	char next;

	while ((buffer[iter] != '\0') && (buffer[iter] != EOF))
	{
		current = buffer[iter];
		next = buffer[iter + 1];

		switch (current)
		{
		//Skip blank spaces and new lines
		case '\t':
		case ' ':
			type = OTHER;
			//iter++;
			continue;

		case ';':
			type = SEMICOLON;
			break;

		case '\n':
			type = NEWLINE;
			break;

		case '|':
			if (current == next)
				type = OR;
			else
				type = PIPE;
			break;

		case '&':
			if (current == next)
			{
				type = AND;
				iter++;
			}
			else
				type = OTHER;
			break;

		case '(':
			type = RIGHT_PAREN;
			break;

		case ')':
			type = LEFT_PAREN;
			break;

		case '<':
			type = LEFT_ARROW;
			break;

		case '>':
			type = RIGHT_ARROW;
			break;

		default:
			type = OTHER;
			break;
		}

		int len = 0;

		token temp;
		temp.type = NULL;

		//Case that the token is a word
		if (isValidChar(current))
		{
			type = WORD;
			//Gets the length of the word within the buffer.
			while (isValidChar(buffer[iter + len]) && buffer[iter + len++] != EOF);

			//Allocates memory for the string stored in the token, accounting for NULL byte at end.
			temp.word = (char *)checked_malloc((len * sizeof(char)) + 1);

			int i;
			//Stores the word from the buffer into the token
			for (i = 0; i < len; i++)
			{
				temp.word[i] = buffer[iter + i];
			}

			//NULL terminates the token
			temp.word[len] = '\0';
			//Increments iter by the length of the string
			iter += len - 1;

		}
		else if (type == OTHER)
		{
			error(1, 0, "Unable to tokenize string");
			exit(1);
		}
		
		
		temp.type = type;
		
		if ( (head->next) == NULL)
		{
			//Store the temp token
			traverse->t = temp;
			//
			traverse->next = (token_list_t)checked_malloc(sizeof(token_list_t));
			traverse = traverse->next;
			traverse->prev = head;
			head->next = traverse;
		}
		else
		{
			traverse->t = temp;
			traverse->next = (token_list_t)checked_malloc(sizeof(token_list_t));
			traverse->prev = traverse;
			traverse = traverse->next;
		}

		iter++;
	}
	
	traverse->next = NULL; 
	return head;
}




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
	/* Method of Implementation
	1.Read the file into a Buffer
	2.Process the buffer into a linked list of commands and operators
	3. Convert list into command trees.
	4. Print Tree's into correct format.
	*/

	//Read the file into the buffer
	char *buffer;
	buffer = create_buffer(get_next_byte, get_next_byte_argument);
	//Process the buffer into a linked list of commands and operators.
	token_list_t list = create_token_list(buffer);
	int iter = 0;
	
	//Test Output
	/*while (list->next != NULL)
	{
		if (list->t.type == WORD)
			printf("%s", list->t.word);
		list = list->next;
	}
	*/
	
	//while (isValidChar(buffer[iter++]));

	//fprintf(stderr, "%d", buffer[--iter]);

	error(1, 0, "command reading not yet implemented");
	return 0;
}







//TODO LATER


/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t
read_command_stream(command_stream_t s)
{

	/* FIXME: Replace this with your implementation too.  */
	error(1, 0, "command reading not yet implemented");
	return 0;
}
