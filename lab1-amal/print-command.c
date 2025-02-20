// UCLA CS 111 Lab 1 command printing, for debugging

#include "command.h"
#include "command-internals.h"

#include <stdio.h>
#include <stdlib.h>


static void single_command_print(command_t c);



static void
command_indented_print (int indent, command_t c)
{
  switch (c->type)
    {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
      {
	//single_command_print(c);
	
	static char const command_label[][3] = { "&&", ";", "||", "|" };

	command_indented_print (indent + 2 * (c->u.command[0]->type != c->type),
				c->u.command[0]);
	printf (" \\\n%*s%s\n", indent, "", command_label[c->type]);
	//fprintf (stderr, " \\\n%*s%s\n", indent, "", command_label[c->type]);
	command_indented_print (indent + 2 * (c->u.command[1]->type != c->type),
				c->u.command[1]);
	break;
      }

    case SIMPLE_COMMAND:
      {
	int i;
	char **w = c->u.word;
	printf ("%*s%s", indent, "", *w);
	//fprintf (stdout, "%*s%s", indent, "", *w);
	//fprintf (stderr, "%*s%s", indent, "", *w);
	//fprintf (stderr, "%*s%s", indent, "", *w);
	/*for (i = 1; i < c->num_words; i++) {
	  fprintf(stderr, " %s", *(w+i));
	  fprintf(stdout, " %s", *(w+i));
	  }*/
	while (*++w) {
	  //printf (" %d", i++);
	  printf (" %s", *w);
	  //fprintf (stderr, " %d", i++);
          //fprintf (stderr, " %s", *w);
	}
	break;
      }

    case SUBSHELL_COMMAND:
      printf ("%*s(\n", indent, "");
      //fprintf (stderr, "%*s(\n", indent, "");
      command_indented_print (indent + 1, c->u.subshell_command);
      printf ("\n%*s)", indent, "");
      //fprintf (stderr, "\n%*s)", indent, "");
      break;

    default:
      abort ();
    }

  if (c->input) {
    printf ("<%s", c->input);
    //fprintf (stderr, "<%s", c->input);
  }
  if (c->output) {
    printf (">%s", c->output);
    //fprintf (stderr, ">%s", c->output);
  }
}

void
print_command (command_t c)
{
  command_indented_print (2, c);
  
  putchar ('\n');
  //fprintf(stderr, "\n");
}
