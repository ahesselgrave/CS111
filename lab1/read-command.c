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

typedef int bool;
#define true 1
#define false 0

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

char operators[7] = {';', '|', '&', '(', ')','<','>'};
bool isOperator(char op){
  int i;
  for (i=0; i<7;i++){
    if (op == operators[i]){
      return true;
    }
  }
  return false;
}

char symbols[11]={'!','%','+',',','-','.','/',':','@','^','_'};
bool isSymbol(char symbol){
  int i;
  for (i =0 ;i<11;i++){
    if(symbol ==symbols[i]){
      return true;
    }
  }
  return false;
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
  int first, second, index = 0, tok_index = 0;
  int linenum = 1;
  // Words, : ! % + , - . / : @ ^ _
  // special tokens: ; | && || ( ) < > 
  
  do
    {
      //decide if should push next char into token- default to true
      bool push_into_token = true;
      first = buffer[index++];
      second = buffer[index];
      printf("initial first: %c, second: %c \n", first,second);
      
      //if letter or digit followed by operator
      if ((isalnum(first)|| isSymbol(first)) && isOperator(second)){
	push_into_token = false;
	//printf("dig-opt first: %c, second: %c\n",first,second);
      }
      
      //if operator followed by letter/digit
      else if (isOperator(first) && (isalnum(second) || isSymbol(second))){
	push_into_token=false;
	//printf("opt-dig first: %c, second: %c\n",first,second);
      }

      else if(isOperator(first) && isspace(second)){
	push_into_token=false;
	//printf("opt-space first: %c, second: %c\n",first,second);
      }
      
      else if (isspace(first) && isOperator(second)){
	push_into_token=false;
	//printf("space-opt first: %c, second: %c\n",first,second);
      }

      else if ((isalnum(first)|| isSymbol(first)|| isspace(first))
	      && second == '\n'){
	push_into_token=false;

	printf("dig-new first: %c, second: %c\n",first,second);
      }

      else if (first =='\n' && second == '\n'){
	push_into_token=false;
	//printf("new-new \n");
      }

      else if ( (isspace(first) && isspace(second)) ||
		isspace(first) && tok_index == 0) {
	push_into_token=false;
      }

      //if first and second characters don't belong together then
      //end current node and make a new one
      if(!push_into_token){
	// if tok_index is still 0 and first is whitespace, don't bother closing the token
	if (isblank(first) && tok_index == 0) {
	  continue;
	}
	//replace consecutive new lines with ;
	else if(first =='\n' && second == '\n'){
	  tok_buf[tok_index++] = ';';
	 
	  //printf("done \n");
	}
	else{
	  //push first char into token
	  tok_buf[tok_index++] = first;
	}
	//end token
	tok_buf[tok_index++] ='\0';
	ts->tail->t = tok_buf;
	//set up new token node
	ts->tail->next = checked_malloc(sizeof(struct token_node));
	ts->tail = ts->tail->next;
	ts->tail->next = NULL;
	//allocate new block for new token and reset token index
	// add second char to  token and increment index so we don't
	tok_index=0;
	tok_buf = checked_malloc(sizeof(char) * token_bufsize);
      }
      else {
	if (first != '\n'){
	  tok_buf[tok_index++] = first;
	}
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
