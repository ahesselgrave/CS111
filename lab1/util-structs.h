#pragma once
#ifndef _UTIL_STRUCTS_H_
#define _UTIL_STRUCTS_H_
#define INITIAL_FILE_VEC_SIZE 128

#include "command.h"
/* queue_t will hold a queue of command_streams that are executed FIFO */
typedef struct queue_t
{
  struct node
  {
    command_stream_t cs;
    struct node *next;
  } *head, *tail;
  struct queue_functions_t *methods;
  int num_nodes;
} queue_t;

/* create a virtual method table for the queue functions*/
struct queue_functions_t
{
  int (*empty)(queue_t *); //returns 1 if empty, 0 otherwise
  int (*size)(queue_t *);
  void (*insert)(queue_t *, command_stream_t); //inserts a command_stream into the queue
  command_stream_t (*pop)(queue_t *); //pops the next command stream in the queue. if empty, returns null
};

void queue_init(queue_t *queue);
int queue_empty(queue_t *q);
int queue_size(queue_t *q);
void queue_insert(queue_t *q , command_stream_t c);
command_stream_t queue_pop(queue_t *q);

typedef struct file_vector
{
  char **filenames;
  int size;
  struct file_vector_methods *methods;
} file_vector_t;

struct file_vector_methods
{
  int (*size)(file_vector_t *);
  int (*empty)(file_vector_t *);
  void (*insert)(file_vector_t *, char *);
  int (*find)(file_vector_t *, char *);
};

void file_vector_init(file_vector_t *);
int file_vector_size(file_vector_t *);
int file_vector_empty(file_vector_t *);
void file_vector_insert(file_vector_t *, char *);
int file_vector_find(file_vector_t *, char *);

#endif
