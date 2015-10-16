// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

//ADDED INCLUDE DIRECTIVES
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

#include<stdio.h>
//ADDED FUNCTION DEFINITIONS
void execute_pipe(command_t c, int time_travel);

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{

  //use series of switch statements for each type of command
  switch (c->type){
  case SIMPLE_COMMAND:
    printf("simple command\n");
    break;
    //case AND_COMMAND:
    //case OR_COMMAND:
    //case SEQUENCE_COMMAND:
  case PIPE_COMMAND:
    printf("pipe command\n");
    execute_pipe(c,time_travel);
    break;
    //case SUBSHELL_COMMAND:
  default:
    printf("none of the given commands\n");
    break;
  }

}

void execute_pipe(command_t c,int time_travel){
  int childStatus;
  pid_t pid, returnPid;
  int infoPipe[2];
  //pipe:unidirectional communication to pass information
  //dup2: used to redirect stuff from write into pipe and push info on read into stdin
  if (pipe(infoPipe)< 0){ //error: can't create pipe
    return;
  }
  
  pid=fork();
  if (pid < 0){ //fork failed
    return;
  }
  //child process
  else if (pid == 0){
    close(infoPipe[0]); //close read end bc not used
    //detect is dup2 succeeds:
    //create copy of write side of pipe and have stdout point to it
    if (dup2(infoPipe[1],1) == -1){
      return;
    }else{
      close(infoPipe[1]); //close write end of pipe (1 = write, 0 = read)
      execute_command(c->u.command[0],time_travel); //go through switch to find type
      //set status
      c->status = c->u.command[0]->status;
    }
  }
  else{//parent process
      close(infoPipe[1]);
      returnPid = waitpid(pid,&childStatus,0); //-1 = wait till child proccess done

      //detect if dup2 succeeded
      if (dup2(infoPipe[0],0) == -1){
	return;
      }else{
	close(infoPipe[0]);
        execute_command(c->u.command[1],time_travel);
	c->status = c->u.command[1]->status;
      }
  }
   /*    //wait till processes complete
    else{
      close(infoPipe[0]);
      close(infoPipe[1]);
      returnPid=waitpid(-1,&childStatus,0);//-1 = wait till any child process
      //is done
      if(returnPid == pid1){
	waitpid(pid2,&childStatus,0);
	c->status=WEXITSTATUS(childStatus);
      }
      else if (returnPid == pid2){
	waitpid(pid2,&childStatus,0);
	c->status= WEXITSTATUS(childStatus);// need to save c->status as child status of second status
	}
    }
    }    */
}
