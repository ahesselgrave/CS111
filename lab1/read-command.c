// UCLA CS 111 Lab 1 command reading

//////////////////////////////////////////////////////////
//
// NOTE: In command_stream generation, we need to delimit
// words with spaces after validating them.
// Also check for backticks
//
/////////////////////////////////////////////////////////

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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

  int numNodes;
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
  END_OF_FILE, //8
  BACKTICK  //9
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
  //stack is full
  if (s->top == 99){
    fprintf(stderr,"Stack is full!");
  }
  else{
    //s->top=s->top + 1;
    s->stk[++s->top]=input;
  }
}

command_t pop(STACK *s){
  if (s->top == -1){
    fprintf(stderr,"Stack is empty!");
    exit(-1);
  }
  else{
    return s->stk[s->top--];
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
      printf("\"%s\"\n", tn->t);
      tn = tn->next;
    }
}

char operators[8] = {';', '|', '&', '(', ')','<','>', '`'};
bool isOperator(char op){
  int i;
  for (i=0; i<8;i++){
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

void
setTokenType(int first, int second, token_stream *ts);
  
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
	  while (first != '\n')
	    {
	      first = buffer[index++];
	      second = buffer[index];
	    }
	  first = second;
	  second= buffer[++index];

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
		(isspace(first) && tok_index == 0)) {
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
  ts->tail->next = NULL;
  ts->tail->tokenType = END_OF_FILE;
  ts->tail->t = "";
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
    if (first == '`'){
      ts->tail->tokenType = BACKTICK;
    }
  }
}

int getPrecedence (token opt){
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
command_stream* sortCommands(token_stream *t_stream){
  struct stack *operatorStack = checked_malloc(sizeof(struct stack));
  struct stack *commandStack = checked_malloc(sizeof(struct stack));
  struct token_node *tokenPointer = t_stream->head;

  //Initialize command_stream and set up initial node that head and tail point to
  struct command_stream *commandStream = checked_malloc(sizeof(struct command_stream));
  commandStream->head = checked_malloc(sizeof(struct command_node));
  commandStream->tail = commandStream->head;
  commandStream->tail->next=NULL;
  commandStream->numNodes = 0;
				       
  //set stacks to be empty
  operatorStack->top=-1;
  commandStack->top=-1;

  //create current and previous command
  command_t currentCmd = checked_malloc(sizeof(struct command));
  // null input and output, modify as needed
  currentCmd->input = NULL;
  currentCmd->output = NULL;
  

  //loop through token linked list to create commands
  while (tokenPointer->tokenType != END_OF_FILE){
    
    //newline -> delimit command trees
    if (tokenPointer ->tokenType == NEWLINE){
      //current command stack is the command node, add to command stream
      //pop off all remaining items from operator stack
      while (sizeOfStack(operatorStack) != 0){
	currentCmd = checked_malloc(sizeof(struct command));
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
  
	command_t operatorLeft= pop(operatorStack);
	command_t rightChild = pop(commandStack);
	command_t leftChild = pop(commandStack);
	currentCmd->type = operatorLeft->type;
	currentCmd->u.command[0]=leftChild;
	currentCmd->u.command[1]=rightChild;
	push(commandStack,currentCmd);
      }
      //add new node to command tree linked list and
      // create new node for next tree
      command_t newCommandTree = pop(commandStack);
      commandStream->tail->command = checked_malloc(sizeof(struct command));
      memcpy(commandStream->tail->command, newCommandTree, sizeof(struct command));
      commandStream->tail->next=checked_malloc(sizeof(struct command_node));
      commandStream->tail = commandStream->tail->next;
      commandStream->tail->next = NULL;
      commandStream->numNodes = commandStream->numNodes +1;
    }

    //not a word
    else if (tokenPointer->tokenType != WORD){
      //if ( push onto stack
      if (strcmp(tokenPointer->t, "(") == 0){
	currentCmd = checked_malloc(sizeof(struct command)); //
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
 
	currentCmd->type = SUBSHELL_COMMAND;
	push(operatorStack,currentCmd);
      }
      //if I/O direction, push onto command stack
      else if (tokenPointer->tokenType == IN_OUT){
	currentCmd = checked_malloc(sizeof(struct command)); //
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
	
	
	currentCmd->type = SIMPLE_COMMAND;
	currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	currentCmd->u.word[0] =(char *) checked_malloc(sizeof(char *));
	currentCmd->u.word[0] = tokenPointer->t;
	currentCmd->u.word[1] = (char *) checked_malloc(sizeof(char *));
	currentCmd->u.word[1] = NULL;
	push(commandStack,currentCmd);
      }
      //if operator stack empty, push onto stack
      else if(operatorStack->top == -1){
	//find type for operator
	currentCmd = checked_malloc(sizeof(struct command));//
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
  

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
	while(!((top(operatorStack))->type == SUBSHELL_COMMAND)){
	  command_t opt= pop(operatorStack);
	  command_t rightCmd = pop(commandStack);
	  command_t leftCmd = pop(commandStack);

	  currentCmd = checked_malloc(sizeof(struct command));//
	  // null input and output, modify as needed
	  currentCmd->input = NULL;
	  currentCmd->output = NULL;
  

	  //create command by combining 2 commands with 1 operator
	  currentCmd->type = opt->type;
	  currentCmd->u.command[0] = leftCmd;
	  currentCmd->u.command[1] = rightCmd;

	  //push back onto command stack
	  push(commandStack,currentCmd);
	}
	//pop matching ( and create subshell command, push back onto stack
	currentCmd = checked_malloc(sizeof(struct command));//
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
  

	command_t firstBracket = pop(operatorStack);
	currentCmd->type= firstBracket->type;  //should be 5?
	command_t subShell= pop(commandStack);
	currentCmd->u.subshell_command=subShell;
	push(commandStack,currentCmd);
      }     
      
      //operator stack not empty- pop all operators greater or equal precedence off operatorStack
      else{
	//check precedence
	int precedenceCurrent = getPrecedence(tokenPointer->t);
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
	while (precedenceCurrent <= precedenceTop){
	  //create command tree and push onto stack
	  currentCmd = checked_malloc(sizeof(struct command));//
	  // null input and output, modify as needed
	  currentCmd->input = NULL;
	  currentCmd->output = NULL;
  

	  command_t opt = pop(operatorStack);
	  command_t rightChild = pop(commandStack);
	  command_t leftChild = pop(commandStack);
	  currentCmd->type = opt->type;
	  currentCmd->u.command[0]=leftChild;
	  currentCmd->u.command[1]=rightChild;
	  push(commandStack,currentCmd);

	  //get value of next operator on stack
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
	      precedenceTop = -1;
	      break;
	    }
	  }
	}
	//push operator onto operatorStack
	int operatorType = tokenPointer->tokenType;
	currentCmd = checked_malloc(sizeof(struct command));//
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
  

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
      //need to check if I/O direction beforehand
      if (sizeOfStack(commandStack) >= 2){
	if ( (strcmp((top(commandStack))->u.word[0],"<") == 0) || (strcmp((top(commandStack))->u.word[0] ,">") ==0)){
	  //pop top two commands and make simple command with input/output redirect

	  //CHANGED HERE!!!! ///////
	  command_t redirect = pop(commandStack);
	  command_t wordForCommand = pop(commandStack);
	  if (wordForCommand->type == SUBSHELL_COMMAND){
	    //wordForCommand -> input = NULL;
	    //wordForCommand->output = NULL;
            // remove trailing spaces at the end
	    char *io = strtok(tokenPointer->t, " ");
	    if (strcmp(redirect->u.word[0],"<") == 0){
	      wordForCommand->input = io;
	      //wordForCommand->u.word = (char **) checked_malloc(sizeof(char **));
	      //wordForCommand->u.word = wordForCommand->u.word;
	    }
	    if (strcmp(redirect->u.word[0],">") == 0){
	      wordForCommand->output = io;
	      //currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	      //currentCmd->u.word = wordForCommand->u.word;
	      
	      // check if wordForCommand has input
	      //if (wordForCommand->input)
	      //currentCmd->input = wordForCommand->input;
	    }
	    push(commandStack,wordForCommand);
	    /*
	    currentCmd->type = SUBSHELL_COMMAND;
	    currentCmd-> input = NULL;
	    currentCmd-> output = NULL;
	    // remove trailing spaces at the end
	    char *io = strtok(tokenPointer->t, " ");
	    if (strcmp(redirect->u.word[0],"<") == 0){
	      currentCmd->input = io;
	      currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	      currentCmd->u.word = wordForCommand->u.word;
	    }
	    if (strcmp(redirect->u.word[0],">") == 0){
	      currentCmd->output = io;
	      currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	      currentCmd->u.word = wordForCommand->u.word;
	      // check if wordForCommand has input
	      if (wordForCommand->input)
		currentCmd->input = wordForCommand->input;
	    }
	    push(commandStack,currentCmd);*/
	  }
	  
	  else{
	  currentCmd = checked_malloc(sizeof(struct command));//
	  // null input and output, modify as needed
	  currentCmd->input = NULL;
	  currentCmd->output = NULL;
  

	  //command_t redirect = pop(commandStack);
	  //command_t wordForCommand = pop(commandStack);
	  currentCmd->type = SIMPLE_COMMAND;
	  // remove trailing spaces at the end
	  char *io = strtok(tokenPointer->t, " ");
	  if (strcmp(redirect->u.word[0],"<") == 0){
	    currentCmd->input = io;
	    currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	    currentCmd->u.word = wordForCommand->u.word;
	  }
	  if (strcmp(redirect->u.word[0],">") == 0){
	    currentCmd->output = io;
	    currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	    currentCmd->u.word = wordForCommand->u.word;
	    // check if wordForCommand has input
	    if (wordForCommand->input)
	      currentCmd->input = wordForCommand->input;
	  }
	  push(commandStack,currentCmd);
	  }
	}
	else{
	  currentCmd = checked_malloc(sizeof(struct command));//
	  currentCmd->type=SIMPLE_COMMAND;
	  int word_index = 0;
	  currentCmd->u.word = (char **)checked_malloc(sizeof(char **));
	  char *c = strtok(tokenPointer->t," ");
	  while (c != 0x0){
	    currentCmd->u.word[word_index]= (char *) checked_malloc(sizeof(char *));
	    currentCmd->u.word[word_index++] =c;
	    c= strtok(NULL, " ");
	  }
	  currentCmd->u.word[word_index] = NULL;
	  push(commandStack,currentCmd);
	}
      }
      else{
	currentCmd = checked_malloc(sizeof(struct command));//
	// null input and output, modify as needed
	currentCmd->input = NULL;
	currentCmd->output = NULL;
  

	currentCmd->type= SIMPLE_COMMAND;
	//use strtok to separate by whitespace for easier validation in future
	int word_index = 0;
	char *c = strtok(tokenPointer->t, " ");
	currentCmd->u.word = (char **) checked_malloc(sizeof(char **));
	while (c != 0x0)
	  {
	    currentCmd->u.word[word_index] = (char *) checked_malloc(sizeof(char *));
	    currentCmd->u.word[word_index++] = c;
	    c = strtok(NULL, " ");
	  }
	currentCmd->u.word[word_index] = NULL;
	push(commandStack,currentCmd);
      }
      
    }
    //after add command/operator to stack, move pointer to next
    tokenPointer= tokenPointer->next;
  }//end of while loop bracket
  return commandStream;
}

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
  int backtick_flag = 0;
  // increments on newline, used for stderr output
  int line_num = 1;

  // assert that first is a word, paren, or backtick initially
  if (first->tokenType != WORD && first->tokenType != PAREN && first->tokenType != BACKTICK)
    is_valid = false;
  
  while(second->next != NULL && is_valid)
    {
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
	case PIPE:
	  // check token length
	  if (type == PIPE)
	    {
	      if (strlen(first->t) != 1)
		{
		  is_valid = false;
		  break;
		}
	    }
	  else if (strlen(first->t) != 2)
	    {
	      is_valid = false;
	      break;
	    }
	  // second can only be the following token types:
	  // WORD, PAREN, NEWLINE
	  if (second_type == WORD ||
	      second_type == PAREN ||
	      second_type == NEWLINE)
	    {
	      if (second_type == NEWLINE)
		{
		  // truncate_newlines(ts, first);
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
		  // check for EOF
		  if (second->tokenType == END_OF_FILE)
		    is_valid = false;
		  continue;
		}
	    }
	  else
    	      is_valid = false;
	  break;
	/* case PIPE: */
	/*   // second cannot be the following token types: */
	/*   // WORD, PAREN, NEWLINE */
	/*   break; */
	case PAREN:
	  // check if right comes before with negative number check
	  // allowed to be undefined
	  if (strcmp(first->t, "((") == 0)
	    exit(-1);
	  else if (strcmp(first->t, "(") == 0)
	    paren_count++;
	  else
	    paren_count--;
	  break;
	case NEWLINE:
	  line_num++;
	  // second_type cant be AND_OR, PIPE, IN_OUT, or SEMICOLON
	  if (second_type == AND_OR || second_type == PIPE ||
	      second_type == IN_OUT || second_type == SEMICOLON)
	    is_valid = false;
	  else
	    {
	      // truncate_newlines(ts, first);
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
	      
	      if (second->tokenType == END_OF_FILE)
		{
		  //EXPLICIT
		  continue;
		}
	      // first = tn;
	      //second = first->next;
	      
	    }
	  break;
	case SEMICOLON:
	  // second can only be the following token types:
	  // WORD, PAREN, NEWLINE
	  if (second_type != WORD && second_type != PAREN && second_type != NEWLINE)
	    is_valid = false;
	  
	  // tokenizing doesnt pick up duplicates like ;;. have to assert size of token
	  // only allow ";"
	  if (strlen(first->t) != 1)
	    is_valid = false;

	  break;
	case IN_OUT:
	  // second can only be the following token types
	  // WORD
	  if (second_type != WORD)
	    is_valid = false;
	  
	  // assert only "<" or ">" tokens by size
	  if (strlen(first->t) != 1)
	    is_valid = false;
	  break;
	case BACKTICK:
	  // only care about having an even number of backticks
	  backtick_flag = !backtick_flag;
	case NONE:
	default:
	  break;
	}

      // this will be negative if right parens came first.
      if (paren_count < 0)
	is_valid = false;
      // set up next iteration
      first = second;
      second = second->next;
    }

  if (paren_count != 0 || !is_valid || backtick_flag != 0)
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
  command_stream* endResult = sortCommands(t_stream);
  free(buffer);
  free(t_stream);
  return endResult;
}

command_t
read_command_stream (command_stream_t s)
{
  //printf("reading command stream here\n");
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  if (s == NULL){
    //printf("nulllllz\n");
    return NULL;
  }
  
  struct command_node *nextCommand = s->head;
  command_t newCommand = nextCommand->command;
  if (newCommand != NULL){
    struct command_node *next = nextCommand->next;
    nextCommand->command = nextCommand->next->command;
    nextCommand->next = nextCommand->next->next;
    //    free(next);
    //printf("meow\n");
  }
  else{
    //printf("moo\n");
    return NULL;
  }
  //  return s->head->command;
  return newCommand;
}
