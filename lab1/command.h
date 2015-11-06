// UCLA CS 111 Lab 1 command interface
#pragma once
#ifndef _COMMAND_H_
#define _COMMAND_H_

// UCLA CS 111 Lab 1 command internals
enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
  int number;
  int level;
};

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;


/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t);

/* Execute a command_stream in parallel */
void execute_parallel (command_stream_t);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);

typedef struct command_stream
{
  struct command_node
  {
    struct command_node *next;
    command_t command;
  } *head, *tail;
  struct command_stream_methods *methods;
  int numNodes;
} command_stream;

struct command_stream_methods
{
  int (*size)(command_stream_t);
  void (*insert)(command_stream_t, command_t);
};

void command_stream_init(command_stream_t);
void command_stream_insert(command_stream_t, command_t);
int command_stream_size(command_stream_t);

#endif
