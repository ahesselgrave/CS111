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

enum token_type{
  WORD,  //0
  AND_OR, //1
  PIPE, //2
  PAREN, //3
  NEWLINE, //4
  SEMICOLON, //5
  IN_OUT, //6
  NONE, //7
};

typedef char *token;
typedef struct token_stream
{
  struct token_node
  {
    enum token_type tokenType;
    token t;
    struct token_node *next;
  } *head, *tail;
} token_stream;

typedef int bool;
#define true 1
#define false 0

struct stack{
  char stk[100];
  int top;
};

typedef struct stack STACK;

void push(STACK *s,char input);
char pop(STACK *s);

void push(STACK *s,char input){
  char val;
  if (s->top == 99){
    //stack is full
    return ' ';
  }
  else{
    s->top=s->top + 1;
    s->stk[s->top]=input;
  }
}

char pop(STACK *s){
  if (s->top == -1){
    return;
  }
  else{
    char val = s->stk[s->top--];
    return val;
  }
}

/*typedef struct{
  char *stk;
  int top;
  int maxSize;
}stack;

void stackInit(stack *s, int maxSize);
void destroyStack(stack *s);
void push(stack *s,char val);
char pop (stack *s);
bool isStackEmpty(stack *s);
bool isStackFull(stack *s);

void stackInit(stack *s, int maxSize){
  printf("making stack");
  char *content;
  content = (char *) malloc(sizeof(char) * maxSize);
  if (content == NULL){
    //not enough space;
    printf("herp\n");
    exit(1);
  }
  s->stk = content;
  s->maxSize = maxSize;
  s->top = -1;
}

void destroyStack(stack *s){
  free(s->stk);
  s->stk = NULL;
  s->maxSize = 0;
  s->top = -1;
}

bool isStackEmpty(stack *s){
  if (s->top == -1){
    return true;
  }
  return false;
}

bool isStackFull(stack *s){
  if(s->top == s->maxSize-1){
    return true;
  }
  return false;
}

void push(stack *s, char val){
  if (!isStackFull(s)){
    s->stk[++s->top] = val;
  }
}

char pop(stack *s){
  if(!isStackEmpty(s)){
    return s->stk[s->top--];
  }
  }*/
  
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
  ts->tail->tokenType = NONE;
  
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
      //      printf("initial first: %c, second: %c \n", first,second);
      
      //if letter or digit followed by operator
      if ((isalnum(first)|| isSymbol(first)) && isOperator(second)){
	push_into_token = false;
      }
      
      //if operator followed by letter/digit
      else if (isOperator(first) && (isalnum(second) || isSymbol(second))){
	push_into_token=false;
      }

      else if(isOperator(first) && isspace(second)){
	push_into_token=false;
      }
      
      else if (isspace(first) && isOperator(second)){
	push_into_token=false;
      }

      else if ((isalnum(first)|| isSymbol(first)|| isspace(first))
	      && second == '\n'){
	push_into_token=false;
      }

      else if (first =='\n' && second == '\n'){
	push_into_token=false;
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
	else{
	  //push first char into token
	  tok_buf[tok_index++] = first;
	}
	//end token
	tok_buf[tok_index++] ='\0';
	ts->tail->t = tok_buf;	
	setTokenType(first,second,ts);
	
	//set up new token node
	ts->tail->next = checked_malloc(sizeof(struct token_node));
	ts->tail = ts->tail->next;
	ts->tail->next = NULL;
	ts->tail->tokenType = NONE;
	//allocate new block for new token and reset token index
	// add second char to  token and increment index so we don't
	tok_index=0;
	tok_buf = checked_malloc(sizeof(char) * token_bufsize);
      }
      else {
	if (first != '\n'){
	  tok_buf[tok_index++] = first;
	  setTokenType(first,second,ts);
	}
      }
    }
  while (first != '\0' && second != '\0');
  //  print_tokens(ts);
  return ts;
}

void setTokenType(int first, int second, token_stream *ts){
  //if token type isn't already set (7=NONE)
  if(ts->tail->tokenType == 7){
    if (isalnum(first) || isSymbol(first)){
      ts->tail->tokenType = 0;
    }
    if ((first == '&' && second =='&') ||
	(first == '|' && second == '|')){
      ts->tail->tokenType = 1;
    }
    if (first =='|' && second != '|'){
      ts->tail->tokenType = 2;
    }
    if (first == '(' || first == ')'){
      ts->tail->tokenType =3;
    }
    if (first == '\n'){
      ts->tail->tokenType = 4;
    }
    if ( first == ';'){
      ts->tail->tokenType = 5;
    }
    if (first == '<' || first == '>'){
      ts->tail->tokenType =6;
    }
    //    printf("\t TOKEN TYPE = %d \n",ts->tail->tokenType);
  }
}

//function that will sort token stream into commands; these commands will
//be inputted as nodes into command_stream 
char* sortCommands(token_stream *t_stream){
  struct stack *operatorStack;
  struct stack *commandStack;
  struct token_node *tokenPointer = t_stream->head;
  
  //loop through token linked list to create commands
  while (tokenPointer->next != NULL){
    printf("%s= %d\n",tokenPointer->t,tokenPointer->tokenType);
    if (tokenPointer->tokenType != 0){
      //if ( push onto stack
      if (tokenPointer->tokenType == 3 && tokenPointer->t=='('){
	push(operatorStack,tokenPointer->t);
      }
      //operator stack empty
      else if(operatorStack->top == -1){
	push(operatorStack,tokenPointer->t);
      }
      //if ), pop all operators off stack until pop (
      else if(tokenPointer->tokenType == 3 && tokenPointer->t==')'){
	pop(operatorStack);
      }
      //operator stack not empty- check for precedence
      //pop all operators greater or equal precedence off opreators stack
      else{
	//check precedence
	pop(operatorStack);
      }
    }
    else{
      //not operator
      push(commandStack,tokenPointer->t);
    }
    tokenPointer= tokenPointer->next;
  }
  //if anything left in operator or command stack, pop operator and combine with
  //2 words to get commands    
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		       void *get_next_byte_argument)
{
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  token_stream *t_stream = tokenize_buffer(buffer);
  //need to create command_stream here and pass into sortCommands as pointer
  char* endResult = sortCommands(t_stream);
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
