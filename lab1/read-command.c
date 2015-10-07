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
  END_OF_FILE //8
};

typedef char *token;
typedef struct token_stream
{
  struct token_node
  {
    enum token_type tokenType;
    token t;
    struct token_node *next, *prev;
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
  char val;
  //stack is full
  if (s->top == 99){
    fprintf(stderr,"Stack is full!");
  }
  else{
    s->top=s->top + 1;
    s->stk[s->top]=input;
  }
}

command_t pop(STACK *s){
  if (s->top == -1){
    fprintf(stderr,"Stack is empty!");
    return;
  }
  else{
    token val = s->stk[s->top--];
    return val;
  }
}

command_t top(STACK *s){
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

      // only need to care about lines that start with '#'
      // can always assume # will be first right after a '\n'
      // always check for comments first
      if (first == '#')
	{
	  // fast forward to end of line
	  while (second != '\n')
	    {
	      first = buffer[index++];
	      second = buffer[index];
	    }
	}
      
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
	ts->tail->next->prev = ts->tail;
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
  // add an EOF token now that we're done
  ts->tail->next = checked_malloc(sizeof(struct token_node));
  ts->tail->next->prev = ts->tail;
  ts->tail = ts->tail->next;
  ts->tail->next = NULL;
  ts->tail->tokenType = END_OF_FILE;
  ts->tail->t = "";
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
  struct stack *operatorStack = checked_malloc(sizeof(struct stack));
  struct stack *commandStack = checked_malloc(sizeof(struct stack));
  struct token_node *tokenPointer = t_stream->head;
  struct command_stream_t *commandStream = checked_malloc(sizeof(command_stream_t));
  //commandStream->head = checked_malloc(sizeof(command_node));
  //commandStream->tail = commandStream->head;

  //set operators to be empty
  operatorStack->top=-1;
  commandStack->top=-1;

  //create current and previous command
  command_t currentCmd;
  command_t prevCmd = NULL;

  //loop through and if hit newline then add to command_stream_t
  
  //loop through token linked list to create commands
  //while (tokenPointer->next != NULL){
  while (tokenPointer->tokenType != 7){
    //    printf("token type= %d\n",tokenPointer->tokenType);

    //newline -> delimit command trees
    if (tokenPointer ->tokenType == 4){
      //current command stack is the command node
      //add to command stream
      printf("new command tree\n");
      command_t newCommandTree = pop(commandStack);
      //commandStream->tail->command = newCommandTree;
      //commandStream->tail->next = NULL;
      printf("size of command stack: %d\n",sizeOfStack(commandStack));
      printf("###################################################\n");
    }

    //not a work
    else if (tokenPointer->tokenType != 0){
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
	currentCmd = checked_malloc(sizeof(struct stack));
	currentCmd->type = SIMPLE_COMMAND;
	currentCmd->u.word = tokenPointer->t;
	push(commandStack,currentCmd);
	printf("sym: %s\n",currentCmd->u.word);
      }
      //if operator stack empty, push onto stack
      else if(operatorStack->top == -1){
	printf("stack empty\n");
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
      if (sizeOfStack(commandStack) >= 2){
	printf("check if need to fix I/O\n");
	if ( (strcmp((top(commandStack))->u.word,"<") == 0) || (strcmp((top(commandStack))->u.word ,">") ==0)){
	  //pop top two commands and make simple command with input/output redirect
	  command_t redirect = pop(commandStack);
	  command_t wordForCommand = pop(commandStack);
	  currentCmd = checked_malloc(sizeof(struct stack));
	  currentCmd->type = SIMPLE_COMMAND;
	  if (strcmp(redirect->u.word,"<") == 0){
	    printf("input\n");
	    currentCmd->input = tokenPointer->t;
	    currentCmd->u.word= &wordForCommand;
	  }
	  if (strcmp(redirect->u.word,">") == 0){
	    printf("output\n");
	    currentCmd->output = tokenPointer->t;
	    currentCmd->u.word= &wordForCommand;
	  }
	  push(commandStack,currentCmd);
	}
      }
      else{
	currentCmd = checked_malloc(sizeof(struct stack));
	currentCmd->type= SIMPLE_COMMAND;
	//use strtok to separate by whitespace for easier validation in future 
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

void
validate(token_stream *ts)
{
  // create two iterators for the linked list
  struct token_node *first = ts->head;
  struct token_node *second;
  if (ts->head->next != NULL)
    second = first->next;

  // is_valid will always be true until flipped
  bool is_valid = true;

  // Increments on '(', decrements on ')'. Should be 0 at the end
  int paren_count = 0;

  // increments on newline, used for stderr output
  int line_num = 1;
  
  while(second->next != NULL && is_valid)
    {
      /* enum token_type{ */
      /* 	WORD,  //0 */
      /* 	AND_OR, //1 */
      /* 	PIPE, //2 */
      /* 	PAREN, //3 */
      /* 	NEWLINE, //4 */
      /* 	SEMICOLON, //5 */
      /* 	IN_OUT, //6 */
      /* 	NONE, //7 */
      /*}; */
      enum token_type type = first->tokenType, second_type = second->tokenType;
      

      /* For binary operators AND_OR, PIPE, we want to fast-forward trailing newlines
	 because they are allowed as many times as we like. For the sake of the 
	 command stream generation, the trailing newlines will be squeezed to a single
         newline that will indicate the end of a command tree.
      */
      switch(type)
	{
	case WORD:
	  // word syntax is at the mercy of the command itself, not our jurisdiction
	  break;
	case AND_OR:
	  // second can only be the following token types:
	  // WORD, PAREN, NEWLINE
	  if (second_type == WORD ||
	      second_type == PAREN ||
	      second_type == NEWLINE)
	    {
	      if (second_type == NEWLINE)
		{
		  // get a copy of where first is
		  struct token_node *tn = first;

		  // fast forward to the next non-newline token in second
		  // don't forget to count line numbers too
		  while (second->tokenType == NEWLINE)
		    {
		      first = second;
		      second = second->next;
		      line_num++;
		    }
		  // remove the newlines from tn to second
		  struct token_node *iter = tn->next, *tmp;
		  while (iter != second)
		    {
		      iter->prev->next = iter->next;
		      iter->next->prev = iter->prev;
		      tmp = iter;
		      iter = iter->next;
		      free(tmp);
		    }

		  // move first and second back to normal
		  // continue to skip the linked list iteration again
		  first = tn;
		  second = first->next;
		  continue;
		}
	    }
	  else
    	      is_valid = false;
	  break;
	case PIPE:
	  // second cannot be the following token types:
	  // WORD, PAREN, NEWLINE
	  break;
	case PAREN:
	  // check if right comes before with negative number check
	  // allowed to be undefined
	  if (strcmp(first->t, "((") == 0)
	    exit(-1);
	  else if (strcmp(first->t, "("))
	    paren_count++;
	  else
	    paren_count--;
	  break;
	case NEWLINE:
	  line_num++;
	  break;
	case SEMICOLON:
	  // second can only be the following token types:
	  // WORD, PAREN, NEWLINE
	  break;
	case IN_OUT:
	  // second can only be the following token types
	  // WORD
	  break;
	case NONE:
	default:
	  break;
	}

      
      // set up next iteration
      first = second;
      second = second->next;
    }

  if (paren_count != 0 || !is_valid)
    {
      fprintf(stderr, "%d: syntax error", line_num);
      exit(1);
    }
    
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		       void *get_next_byte_argument)
{  
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  token_stream *t_stream = tokenize_buffer(buffer);
  // validate the buffer: print to stderr if incorrect and exit(1)
  validate(t_stream);
  //need to create command_stream here and pass into sortCommands as pointer
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  //continue calling sortCommands(t_stream) until no more command nodes
  //or make command_stream_t in sortCommands and return it
  char* endResult = sortCommands(t_stream);
  free(buffer);
  free(t_stream);
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  /* if (s == NULL){
    printf("nulllllz\n");
    return NULL;
  }
  struct command_node *nextCommand = s->head;
  command_t newCommand = nextCommand->command;
  //  command_t newCommand = s->head->command;
  if (nextCommand->next != NULL){
    command_stream_t next = s->next;
    s->command = s->next->command;
    s->next = s->next->next;
    free(next);
  }
  else{
    s->command = NULL;
  }
  return newCommand;*/
  return;
}
