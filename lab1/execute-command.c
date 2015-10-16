// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

#define STDIN 0
#define STDOUT 1

int
command_status (command_t c)
{
  return c->status;
}

// Set up function prototypes
void execute_simple(command_t c);
void execute_and(command_t c);
void execute_or(command_t c);
void execute_sequence(command_t c);
void execute_pipe(command_t c);
void execute_subshell(command_t c);

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

  //use series of switch statements for each type of command
  switch (c->type){
  case SIMPLE_COMMAND:
    execute_simple(c);
    break;
    
  /* case AND_COMMAND: */
  /*   execute_and(c); */
  /*   break; */
    
  /* case OR_COMMAND: */
  /*   execute_or(c); */
  /*   break; */
    
  /* case SEQUENCE_COMMAND: */
  /*   execute_sequence(c); */
  /*   break; */
    
  /* case PIPE_COMMAND: */
  /*   execute_pipe(c); */
  /*   break; */

  /* case SUBSHELL_COMMAND: */
  /*   execute_subshell(c); */
  /*   break; */
  default:
    //How did we get here?
    error (1, 0, "YOU BROKE IT GOOD JOB!");
  }
  // avoid compiler warning with -Werror on
  time_travel;
}

void
execute_simple(command_t c)
{
  // Fork shell and execvp
  int status;
  pid_t p = fork();

  if (p == 0) //child
    {
      //check for input and output
      if (c->input)
	{
	  // open the file in command input and clone the fd into stdin
	  int in = open(c->input, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	  dup2(in, STDIN);
	  close(in);
	  
	}
      if (c->output)
	{
	  int out = open(c->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	  dup2(out, STDOUT);
	  close(out);
	}
      // guess two arguments, realloc if needed
      int num_args = 2;
      int index = 0;
      char **argv = (char **) checked_malloc(sizeof(char *) * num_args);
      char **w = c->u.word;
      while(*w)
	{
	  size_t len = strlen(*w);
	  argv[index] = (char *) checked_malloc(sizeof(char) * len);
	  strcpy(argv[index++], *w);
	  //realloc if needed
	  if (index > num_args)
	    {
	      num_args *= 2;
	      argv = (char **) checked_realloc(argv, sizeof(char*) * num_args);
	    }
	  w++;
	}
      //add null termination
      if (index > num_args)
	{
	  num_args *=2;
	  argv = (char **) checked_realloc(argv, sizeof(char) * num_args);
	}
      argv[index] = NULL;
      
      if (execvp(*argv, argv) < 0)
	error(1, 0, "ERROR IN EXECVP");
	  
    }
  else if (p > 0) //parent
    {
      //hang up shell until done
      while( wait(&status) != p);
    }
  else //error
    error (1, 0, "Error in fork!");
}


void
execute_pipe(command_t c){
  int timetravel=0;
  int childStatus;
  pid_t pid1, pid2, returnPid;
  int mypipe[2];
  if (pipe(mypipe)< 0){
    //error
    return;
  }
  pid1=fork();
  //child process
  if (pid1 == 0){
    close(mypipe[0]); //close read end bc not used
    dup2(mypipe[1],1);//redirect stdout to write end of pipe
    //detect is dup2 suceeds
    close(mypipe[1]); //close write end of pipe
    execute_command(c->u.command[0],timetravel); //go through switch to find type
  }
  else{//parent process
    pid2=fork();
    if(pid2 == 0){
      close(mypipe[1]);
      dup2(mypipe[0],0);
      //detect if dup2 succeeded
      close(mypipe[0]);
      execute_command(c->u.command[1],timetravel);
    }
    else{
      close(mypipe[0]);
      close(mypipe[1]);
      returnPid=waitpid(-1,&childStatus,0);//-1 = wait till any child process
      //is done
      if(returnPid == pid1){
	waitpid(pid2,&childStatus,0);
	c->status=WEXITSTATUS(childStatus);
      }
      else if (returnPid == pid2){
	//similar to above but must save c->status as child status of second
	//status
      }
    }
  }    
}
