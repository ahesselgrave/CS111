// UCLA CS 111 Lab 1 command reading

//////////////////////////////////////////////////////////
//
// NOTE: In command_stream generation, we need to delimit
// words with spaces after validating them.
// Also check for backticks
//
/////////////////////////////////////////////////////////

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
  char stk[100];
  int top;
};

typedef struct stack STACK;

void push(STACK *s,char input);
char pop(STACK *s);
char top(STACK *s);

void push(STACK *s,char input){
  //printf("pushing\n");
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

char top(STACK *s){
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
    //    printf("\t TOKEN TYPE = %d \n",ts->tail->tokenType);
    }
  }
}

int getPrecedence (token opt){
  //printf("token: %s\n", opt);
  if (opt == ";" || opt == "\n"){
    return 0;
  }
  else if (opt == "&&" || opt == "||"){
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
  operatorStack->top=-1;
  commandStack->top=-1;
  //loop through token linked list to create commands
  while (tokenPointer->next != NULL){
    //printf("%s= %d\n",tokenPointer->t,tokenPointer->tokenType);
    if (tokenPointer->tokenType != 0){
      //if ( push onto stack
      //strcmp returns 0 when they match
      if (tokenPointer->tokenType == 3 && strcmp(tokenPointer->t, "(") == 0){
	//printf("push (\n");
	push(operatorStack,tokenPointer->t);
      }
      //operator stack empty
      else if(operatorStack->top == -1){
	//printf("opt stack empty\n");
	push(operatorStack,tokenPointer->t);
      }
      //if ), pop all operators off stack until pop (
      else if(tokenPointer->tokenType == 3 && strcmp(tokenPointer->t, ")") == 0){
	//printf(") so pop\n");
	pop(operatorStack);
      }
      //if newline, increment lineNum
      /*else if (tokenPointer ->tokenType == 4){
	printf("new line!\n");
	lineNum++;
	}*/
      
      //operator stack not empty- check for precedence
      //pop all operators greater or equal precedence off opreators stack
      else{
	//check precedence
	int precedenceCurrent = getPrecedence(tokenPointer->t);
	//	printf("precedence:%d\n",precedenceCurrent);
	//int precedenceTop = getPrecedence(top(operatorStack));
	/*while(precedenceTop >= precedenceCurrent){
	  pop(operatorStack);
	  precedenceTop = getPrecedence(top(operatorStack));
	  //make command trees here
	  }*/
	
	//	printf("p1= %d, p2=%d\n",precedenceCurrent, precedenceTop);
      }
      
    }
    else{
      //not operator
      //      printf("push word\n");
      push(commandStack,tokenPointer->t);
    }
    tokenPointer= tokenPointer->next;
  }
  //if anything left in operator or command stack, pop operator & combine with
  //2 words to get commands    
}

void
truncate_newlines(token_stream *ts, struct token_node *first, int *line_num)
{
  // get a copy of where first is
  struct token_node *tn = first, *second = first->next;
  
  // fast forward to the next non-newline token in second
  // don't forget to count line numbers too
  while (second->tokenType == NEWLINE)
    {
      first = second;
      second = second->next;
      (*line_num)++;
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

  // assert that first is a word initially
  if (first->tokenType != WORD)
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
	  if (type == PIPE && strlen(first->t) != 1)
	    {
	      is_valid = false;
	      break;
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
	  else if (strcmp(first->t, "("))
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
  token opt = " ;";
  int x = getPrecedence(opt);
  //  printf("VALUE: %d\n",x);
  
  char *buffer = read_from_input(get_next_byte, get_next_byte_argument);
  token_stream *t_stream = tokenize_buffer(buffer);
  // validate the buffer: print to stderr if incorrect and exit(1)
  validate(t_stream);
  //need to create command_stream here and pass into sortCommands as pointer
  //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
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
