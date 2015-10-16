// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //shuts up compiler

  //use series of switch statements for each type of command
  switch (c->type){
    //case SIMPLE_COMMAND:
    //case AND_COMMAND:
    //case OR_COMMAND:
    //case SEQUENCE_COMMAND:
  case PIPE_COMMAND:
    execute_pipe(c);
    //case SUBSHELL_COMMAND:
    //default:
    
  }
  c;
  time_travel;
  error (1, 0, "command execution not yet implemented");
}

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
