// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <ctype.h>
#include <stdio.h>

#define TT_BUFFER_SIZE 1024

// command_stream holds a linked list of command trees, with each node's command_t being the root pointer
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

char *
read_from_input(int (*get_char) (void *),
		void *get_char_arg)
{
  int bufsize = TT_BUFFER_SIZE;
  int index = 0;
  char *buffer = checked_malloc(sizeof(char) * bufsize);

  int c;
  while ((c = get_char(get_char_arg)) != EOF)
    {
      // Put c into the buffer and make sure it wont extend past memory
      buffer[index++] = c;
      if (index > bufsize) {
	bufsize += TT_BUFFER_SIZE;
	buffer = checked_realloc(buffer, bufsize);
      }
    }
  return buffer;
}
       
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		       void *get_next_byte_argument)
{
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  printf("%s", buffer);
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  // error (1, 0, "command reading not yet implemented");
  return 0;
}
