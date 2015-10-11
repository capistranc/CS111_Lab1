// UCLA CS 111 Lab 1 command internals
typedef enum token_type
{
	WORD, //0
	SEMICOLON, //1
	PIPE, //2
	AND, //3
	OR, //4
	LEFT_PAREN, //5
	RIGHT_PAREN, //6
	LEFT_ARROW, //7
	RIGHT_ARROW, //8
	NEWLINE, //9
	OTHER, //10
	ENDTREE //11
}token_type;

enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;
  token_type tok_type;
  int line;
  int pos;

  
  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};
