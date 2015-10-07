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
  command_t stk[100];
  int top;
};

typedef struct stack STACK;

void push(STACK *s,command_t input);
command_t pop(STACK *s);
command_t top(STACK *s);
int sizeOfStack(STACK *s);

int sizeOfStack(STACK *s){
  return s->top+1;
}

void push(STACK *s,command_t input){
  //  printf("pushing\n");
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

command_t pop(STACK *s){
  if (s->top == -1){
    return;
  }
  else{
    token val = s->stk[s->top--];
    return val;
  }
}

command_t top(STACK *s){
  /*printf("top of stack\n");
  printf("top num =%d\n",s->top);
  printf("%s\n",s->stk[s->top]);*/
  return s->stk[s->top];
}

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
      //add \n
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
  print_tokens(ts);
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

int getPrecedence (token opt){
  printf("token: %s\n", opt);
  if ((strcmp(opt,";") == 0) || (strcmp(opt,"\n") == 0)){
    return 0;
  }
  else if ((strcmp(opt,"&&") == 0) || (strcmp(opt,"||") == 0)){
    return 1;
  }
  else{ //pipe
    return 2;
  }
}
  

//function that will sort token stream into commands; these commands will
//be inputted as nodes into command_stream 
char* sortCommands(token_stream *t_stream){
  int lineNum = 1;
  struct stack *operatorStack = checked_malloc(sizeof(struct stack));
  struct stack *commandStack = checked_malloc(sizeof(struct stack));
  struct token_node *tokenPointer = t_stream->head;

  //set operators to be empty
  operatorStack->top=-1;
  commandStack->top=-1;

  //create current and previous command
  command_t currentCmd;
  command_t prevCmd = NULL;
  //loop through token linked list to create commands
  while (tokenPointer->next != NULL){
    //if not a word
    if (tokenPointer->tokenType != 0){
      //if ( push onto stack
      if (strcmp(tokenPointer->t, "(") == 0){
	printf("( BRACKET\n");
	currentCmd = checked_malloc(sizeof( struct stack));
	currentCmd->type = SUBSHELL_COMMAND;
	push(operatorStack,currentCmd);
	//printf("size of stack: %d\n",sizeOfStack(operatorStack));
	//printf("value: %d\n",(top(operatorStack))->type);
      }
      //if I/O direction, push onto command stack
      else if (tokenPointer->tokenType == 6){
	printf("input/output\n");
	currentCmd = checked_malloc(sizeof(struct stack));
	currentCmd->type = SIMPLE_COMMAND;
	currentCmd->u.word = tokenPointer->t;
	push(commandStack,currentCmd);
	/*printf("size of stack: %d\n",sizeOfStack(commandStack));*/
	  printf("value: %d\n",(top(commandStack))->type);                                                                                                                                                      
	  printf("word: %s\n",currentCmd->u.word);
      }
      //if operator stack empty, push onto stack
      else if(operatorStack->top == -1){
	printf("stack empty\n");
	//	printf("value: %d\n",tokenPointer->tokenType);
	currentCmd = checked_malloc(sizeof(struct stack));
	//find type for operator
	int operatorType = tokenPointer->tokenType;
	switch (operatorType){
	case 1:
	    if(strcmp(tokenPointer->t,"&&") == 0){
	      currentCmd->type=AND_COMMAND;
	    }
	    else{
	      currentCmd->type=OR_COMMAND;
	    }
	    break;
	case 2:
	  currentCmd->type=PIPE_COMMAND;
	  break;
	case 5:
	  currentCmd->type=SEQUENCE_COMMAND;
	  break;
	default:
	  //error because should be one of the above
	  break;
	}
	push(operatorStack,currentCmd);
      }
      
      //if ), pop all operators off stack until pop (
      else if(strcmp(tokenPointer->t, ")") == 0){
	//keep popping as until reach )
	printf("close parens\n");
	while(!((top(operatorStack))->type == 5)){
	  command_t opt= pop(operatorStack);
	  command_t rightCmd = pop(commandStack);
	  command_t leftCmd = pop(commandStack);
	  currentCmd = checked_malloc(sizeof(struct stack));
	  
	  //create command by combining 2 commands with 1 operator
	  currentCmd->type = opt->type;
	  currentCmd->u.command[0] = leftCmd;
	  currentCmd->u.command[1] = rightCmd;

	  //push back onto command stack
	  push(commandStack,currentCmd);
	  printf("pushed in close paren\n");
	}
	//pop matching ( and create subshell command, push back onto stack
	command_t firstBracket = pop(operatorStack);
	currentCmd->type= firstBracket->type;  //should be 5?
	command_t subShell= pop(commandStack);
	currentCmd->u.subshell_command=subShell;
	push(commandStack,currentCmd);
	printf("removed paren\n");
      }     
      
      //operator stack not empty- pop all operators greater or equal precedence off operatorStack
      else{
	//check precedence
	int precedenceCurrent = getPrecedence(tokenPointer->t);
	printf("cur: %d\n",precedenceCurrent);
	command_t prevOperator = top(operatorStack);
	int precedenceTop = -1;
	switch (prevOperator->type){
	case AND_COMMAND:
	case OR_COMMAND:
	  precedenceTop = 1;
	  break;
	case SEQUENCE_COMMAND:
	  precedenceTop = 0;
	  break;
	case PIPE_COMMAND:
	  precedenceTop = 2;
	  break;
	default:
	  break;
	}
	//	printf("top: %d\n",precedenceTop);
	currentCmd = checked_malloc(sizeof(struct stack));
	while (precedenceCurrent <= precedenceTop){
	  //printf("should create new tree\n");

	  //create command tree and push onto stack
	  command_t opt = pop(operatorStack);
	  command_t rightChild = pop(commandStack);
	  command_t leftChild = pop(commandStack);
	  currentCmd->type = opt->type;
	  currentCmd->u.command[0]=leftChild;
	  currentCmd->u.command[1]=rightChild;
	  push(commandStack,currentCmd);

	  //get value of next operator on stack
	  //printf("current operator value: %d\n",currentCmd->type);
	  if (sizeOfStack(operatorStack) == 0){
	    break;
	  }
	  else{
	    switch ((top(operatorStack))->type){
	    case AND_COMMAND:
	    case OR_COMMAND:
	      precedenceTop = 1;
	      break;
	    case SEQUENCE_COMMAND:
	      precedenceTop = 0;
	      break;
	    case PIPE_COMMAND:
	      precedenceTop = 2;
	      break;
	    default:
	      break;
	    }
	  }
	}
	//push operator onto operatorStack
	int operatorType = tokenPointer->tokenType;
	switch (operatorType){
	case 1:
	  if(strcmp(tokenPointer->t,"&&") == 0){
	    currentCmd->type=AND_COMMAND;
	  }
	  else{
	    currentCmd->type=OR_COMMAND;
	  }
	  break;
	case 2:
	  currentCmd->type=PIPE_COMMAND;
	  break;
	case 5:
	  currentCmd->type=SEQUENCE_COMMAND;
	  break;
	default:
	  //error because should be one of the above
	  break;
	}	
	push(operatorStack,currentCmd);
      }
    }
    //if not operator
    else{
      printf("word\n");
      //need to check if I/O direction beforehand
      if (sizeOfStack(commandStack) != 0){
	printf("check if need to fix I/O\n");
	printf("meow: %s\n",(top(commandStack))->u.word);
	if ( (strcmp((top(commandStack))->u.word,"<") == 0) || (strcmp((top(commandStack))->u.word ,">") ==0)){
	  printf("redirection, need to create simple command!\n");
	}
      }
      else{
	currentCmd = checked_malloc(sizeof(struct stack));
	currentCmd->type= SIMPLE_COMMAND;
	currentCmd->u.word = tokenPointer->t;
	push(commandStack,currentCmd);
      }
      /*printf("size of stack: %d\n",sizeOfStack(commandStack));
      printf("value: %d\n",(top(commandStack))->type);
      printf("word: %s\n",currentCmd->u.word);*/
      
    }
    //after add command/operator to stack, move pointer to next
    printf("operatorStack size: %d, commandStack size %d\n",sizeOfStack(operatorStack),sizeOfStack(commandStack));
    tokenPointer= tokenPointer->next;
    prevCmd = currentCmd;
  }//end of while loop bracket
}

  //if anything left in operator or command stack, pop operator & combine with
  //2 words to get commands    

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		       void *get_next_byte_argument)
{  
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  token_stream *t_stream = tokenize_buffer(buffer);
  //need to create command_stream here and pass into sortCommands as pointer
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
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
  //if end of stream, return nothing
  //else return s->command
  return 0;
}
