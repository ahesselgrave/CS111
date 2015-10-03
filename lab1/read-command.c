// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <ctype.h>
#include <stdio.h>
/* Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

// Define a linked list of commands
typedef struct command_stream
{
  struct command_node
  {
    struct command_node *next;
    command_t command;
  } *head;
} command_stream;

// Tokens are:
// ; | && || ( ) < >
// Comments are:
// #
// Treat as regular char:
// ! % + , - . / : @ ^ _

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // Read two characters at a time to find two character tokens
  int first, second;
  // Other spatulers
  int linenum = 1,
    valid_syntax = 0;

  // Initialize first and second before while looping
  first = get_next_byte(get_next_byte_argument);
  second = get_next_byte(get_next_byte_argument);

  while (second != EOF && valid_syntax == 0) {
    // For now, make sure we are getting the input without segfaulting
    putchar(first);
    if (first == '\n') linenum++;

    // Get next chars at the end
    first = second;
    second = get_next_byte(get_next_byte_argument);
  }
  putchar(first);
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  // error (1, 0, "command reading not yet implemented");
  return 0;
}
