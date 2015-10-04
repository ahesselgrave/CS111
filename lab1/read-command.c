
// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define TT_BUFFER_SIZE 1024
#define TT_TOKEN_BUFSIZE 128

// command_stream holds a linked list of command trees, with each node's command_t being the root pointer
typedef struct command_stream
{
  struct command_node
  {
    struct command_node *next;
    command_t command;
  } *head, *tail;
} command_stream;

typedef char *token;
typedef struct token_stream
{
  struct token_node
  {
    token t;
    struct token_node *next;
  } *head, *tail;
} token_stream;

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
      if (index > bufsize)
	{
	  bufsize += TT_BUFFER_SIZE;
	  buffer = checked_realloc(buffer, bufsize);
	}
    }
  // Don't forget the zero byte at the end
  buffer[index] = '\0';
  return buffer;
}

void
print_tokens(token_stream *ts)
{
  struct token_node *tn = ts->head;
  while (tn->next != NULL)
    {
      printf("%s\n", tn->t);
      tn = tn->next;
    }
}

token_stream *
tokenize_buffer(char *buffer)
{
  int token_bufsize = TT_TOKEN_BUFSIZE;

  // Initialize token_stream and set up an initial node that head and tail point to
  token_stream *ts = checked_malloc(sizeof(token_stream));
  ts->head = checked_malloc(sizeof(struct token_node));
  ts->tail = ts->head;
  
  token tok_buf = checked_malloc(sizeof(char) * token_bufsize);
  // Read 2 chars at a time.
  // Initialize them to the first two characters.
  // Safe to assume from test cases that there is no 1 char buffer.
  int first, second, index = 0, tok_index = 0;
  int linenum = 1;
  // Words, : ! % + , - . / : @ ^ _
  // special tokens: ; | && || ( ) < > 
  
  do
    {
      first = buffer[index++];
      if (first != ' ' && first != '\t')
	tok_buf[tok_index++] = first;
      second = buffer[index];

      switch(second)
	{
	case ';':
	  // end token and put it in da linked list
	  // disregard memory, leaks aren't checked
	  tok_buf[tok_index++] = '\0';
	  ts->tail->t = tok_buf;
	  ts->tail->next = checked_malloc(sizeof(struct token_node));
	  ts->tail = ts->tail->next;
	  ts->tail->next = NULL;

	  // add semicolon token and increment index so we don't
	  // put semicolon in another token
	  ts->tail->t = ";";
	  ts->tail->next = checked_malloc(sizeof(struct token_node));
	  ts->tail = ts->tail->next;
	  ts->tail->next = NULL;
	  index++;
	  
	  // allocate a new block for a new token and reset token index
	  tok_index = 0;
	  tok_buf = checked_malloc(sizeof(char) * token_bufsize);
	  break;
	case '|':
	  break;
	case '&':
      	  break;
	case '(':
	  break;
	case ')':
	  break;
	case '<':
	  break;
	case '>':
	  break;
	case '#':
	  break;
	case '\n':
	  linenum++;
	  break;
	default:
	  break;
	}
    }
  while (first != '\0' && second != '\0');
  print_tokens(ts);
  return 0;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		       void *get_next_byte_argument)
{
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  token_stream *t_stream = tokenize_buffer(buffer);

  free(buffer);
  free(t_stream);
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  // error (1, 0, "command reading not yet implemented");
  return 0;
}
