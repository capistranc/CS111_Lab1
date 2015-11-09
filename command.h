// UCLA CS 111 Lab 1 command interface

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

//User Added Functions
char* make_buffer(int(*get_next_byte) (void *), void *get_next_byte_argument);

struct linked_list* create_token_list(char* buffer);

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the commandmake
 stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

int length_command_stream(command_stream_t);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);

/* User added functions*/
int** create_dependecy_graph(command_stream_t command_stream, command_t* forrest);
void executeTimeTravel(command_t* forrest, int** dependency_graph, int num_cmds);

void run_ready_commands(command_t *forrest, int** dependency_graph, int* has_dependency, int* executable, int num_cmds);

