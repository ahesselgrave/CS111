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
#include <stdio.h>
#include <stdlib.h>
#define STDIN 0
#define STDOUT 1

int
command_status (command_t c)
{
  return c->status % 255;
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
  case AND_COMMAND:
    execute_and(c);
    break;

  case OR_COMMAND:
    execute_or(c);
    break;

  case SEQUENCE_COMMAND: 
    execute_sequence(c); 
    break; 

  case PIPE_COMMAND:
    execute_pipe(c);
    break; 

  case SUBSHELL_COMMAND:
    execute_subshell(c);
    break;    
    
  default:
    //How did we get here?
    error(1,0,"YOU BROKE IT GOOD JOB!");
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
	{
	  fprintf(stderr, "timetrash: %s: command not found\n", *argv);
	  exit(1);
	}
	  
    }
  else if (p > 0) //parent
    {
      waitpid(p, &status, 0);
      c->status = status % 255;
    }
  else //error
    error (1, 0, "Error in fork!");
}

void
execute_and(command_t c)
{
  int time_travel = 0;
  int child1_status, child2_status;
  pid_t pid1, pid2;

  pid1 = fork();
  if (pid1 < 0)
    error(1,0,"Error forking left operand of &&");
  else if (pid1 == 0)
    {
      // This executes the left operand, which will always execute
      execute_command(c->u.command[0], time_travel);
      exit(command_status(c->u.command[0]));
    }
  else
    {
      waitpid(pid1, &child1_status, 0);
      if (child1_status == 0)
	{
	  pid2 = fork();
	  if (pid2 < 0)
	    error(1,0, "Error forking right operand of &&");
	  else if (pid2 == 0)
	    {
	      //Execute right operand
	      execute_command(c->u.command[1], time_travel);
	      exit(command_status(c->u.command[1]));
	    }
	  else
	    {
	      waitpid(pid2, &child2_status, 0);
	      c->status = child2_status % 255;
	    }
	}
      else
	{
	  c->status = child1_status;
	  return;
	}
    }
}
void
execute_or(command_t c)
{
  int time_travel = 0;
  int child1_status, child2_status;
  pid_t pid1, pid2;

  pid1 = fork();
  if (pid1 < 0)
    error(1,0,"Error forking left operand of &&");
  else if (pid1 == 0)
    {
      // This executes the left operand, which will always execute
      execute_command(c->u.command[0], time_travel);
      exit(command_status(c->u.command[0]));
    }
  else
    {
      waitpid(pid1, &child1_status, 0);
      if (child1_status != 0)
	{
	  pid2 = fork();
	  if (pid2 < 0)
	    error(1,0, "Error forking right operand of &&");
	  else if (pid2 == 0)
	    {
	      //Execute right operand
	      execute_command(c->u.command[1], time_travel);
	      exit(command_status(c->u.command[1]));
	    }
	  else
	    {
	      waitpid(pid2, &child2_status, 0);
	      c->status = child2_status % 255;
	    }
	}
      else
	{
	  c->status = child1_status;
	  return;
	}
    }
}

void
execute_pipe(command_t c){
  int timetravel=0;
  int childStatus;
  //pid_t pid1, pid2, returnPid;
  pid_t pid;
  int infoPipe[2];
  
  if (pipe(infoPipe)< 0){
    error(1,0,"ERROR WITH PIPE");
  }
  
  pid = fork();
  if (pid < 0){
    error(1,0,"Error in forking");
  }
  //child process executes first command
  else if (pid == 0){
    close(infoPipe[0]);
    if (dup2(infoPipe[1],1) == -1){
      error(1,0,"ERROR WITH DUP2");
    }
    close(infoPipe[1]);
    execute_command(c->u.command[0],timetravel);
    c->status = c->u.command[0]->status;
    exit(command_status(c));
  }
  //parent executes right side of pipe
  else{
    close(infoPipe[1]);
    waitpid(0,&childStatus,0);
    if (dup2(infoPipe[0],0) == -1){
      error(1,0,"ERROR WITH DUP2");
    }
    close(infoPipe[0]);
    execute_command(c->u.command[1],timetravel);
    c->status = c->u.command[1]->status;
  }
}
 
void
execute_sequence(command_t c){
  int timetravel = 0;
  command_t firstC = c->u.command[0];
  command_t secondC = c->u.command[1];
  
  int status;
  pid_t pid;
  pid= fork();
  if (pid > 0){
    waitpid(0,&status,0);
    c->status = status %255;
  }
  else if (pid == 0){
    pid = fork();
    if (pid > 0){
      waitpid(0,&status,0);
      execute_command(secondC,timetravel);
      //_exit terminates calling process and argument is returned to parent
      //process as exit status
      exit(command_status(secondC));
    }
    else if (pid == 0){
      execute_command(firstC, timetravel);
      _exit(command_status(firstC));
    }
    else{
      error(1,0,"error in forking");
    }
  }
  else{
    error(1,0,"error in forking");
  }
  }

/*
void
execute_sequence(command_t c){
  int timetravel = 0;
  command_t first = c->u.command[0];
  command_t second = c->u.command[1];
  int status;
  pid_t pid;
  pid = fork();
  if (pid > 0){
    waitpid(0,&status,0);
    //c->status = status;
    c->status = second->status;
  }
  else if (pid == 0){
    pid = fork();
    if (pid < 0 ){
      error(1,0,"error");
    }
    else if (pid == 0){
      execute_command(first,timetravel);
      c->status=first->status;
      exit(first->status);
    }
    else{
      waitpid(pid,&status,0);
      execute_command(second,timetravel);
      c->status=second->status;
      exit(second->status);
    }
  }
  else{
    error(1,0,"error forking");
  }
}
*/
void
execute_subshell(command_t c)
{
  int time_travel = 0;
  int status;
  pid_t pid;
  pid = fork();
  if (pid < 0)
    error(1,0,"Error forking in subshell");
  else if (pid == 0)
    {
      execute_command(c->u.subshell_command, time_travel);
      exit(command_status(c->u.subshell_command));
    }
  else
    {
      waitpid(pid, &status, 0);
      c->status = status % 255;
    }
}
