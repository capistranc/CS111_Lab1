// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>

#include <getopt.h>
#include <stdio.h>

#include "command.h"

#include "alloc.h"
#include "command-internals.h"
#include <unistd.h>

#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for waitpid
#include <unistd.h> // for execvp, fork
#include <stdio.h> // for fprintf
#include <stdlib.h> // for exit
#include <fcntl.h> // for file open constants

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = 1; break;
      case 't': time_travel = 1; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;
  
  if (time_travel == 1)
  {
    int num_cmds = length_command_stream(command_stream);
    command_t *forrest = (command_t*)malloc(sizeof(command_t)*num_cmds);
    int** matrix = create_dependecy_graph(command_stream, forrest);
	  
    executeTimeTravel(forrest, matrix, num_cmds);

  }
  while ((command = read_command_stream (command_stream)))
    {
      if (print_tree)
	{
	  printf ("# %d\n", command_number++);
	  print_command (command);
	}
      else
	{
	  fprintf(stderr, "mainCHECKER %d\t", command->type);
	  last_command = command;
	  execute_command (command);
	}
    }

  return print_tree || !last_command ? 0 : command_status (last_command);
}
