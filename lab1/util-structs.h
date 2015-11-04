#ifndef UTIL_STRUCTS_H
#define UTIL_STRUCTS_H

#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_FILE_VEC_SIZE 128
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
  void (*insert)(command_stream_t, queue_t *); //inserts a command_stream into the queue
  command_stream_t (*pop)(queue_t *); //pops the next command stream in the queue. if empty, returns null
};

int queue_empty(queue_t *q);
int queue_size(queue_t *q);
void queue_insert(command_stream_t c, queue_t *q);
command_stream_t queue_pop(queue_t *q);

void
queue_init(queue_t *queue)
{
  queue->num_nodes = 0;
  queue->head = checked_malloc(sizeof(struct node));
  queue->head->next = NULL;
  queue->tail = queue->head;
  queue->methods = checked_malloc(sizeof(struct queue_functions_t));
  queue->methods->empty = queue_empty;
  queue->methods->size = queue_size;
  queue->methods->insert = queue_insert;
  queue->methods->pop = queue_pop;
}

int
queue_empty(queue_t *q)
{
  return q->num_nodes == 0 ? 1 : 0;
}

int
queue_size(queue_t *q)
{
  return q->num_nodes;
}

void
queue_insert(command_stream_t c, queue_t *q)
{
  q->tail->cs = c;
  q->tail->next = checked_malloc(sizeof(struct node));
  q->tail = q->tail->next;
  q->tail->next = NULL;
  q->num_nodes++;
}

command_stream_t
queue_pop(queue_t *q)
{
  command_stream_t cs = q->head->cs;
  struct node *temp = q->head;
  q->head = q->head->next;
  q->num_nodes--;
  free(temp);
  return cs;
}

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

int file_vector_size(file_vector_t *);
int file_vector_empty(file_vector_t *);
void file_vector_insert(file_vector_t *, char *);
int file_vector_find(file_vector_t *, char *);

void
file_vector_init(file_vector_t *f)
{
  f->filenames = (char **) checked_malloc(sizeof(char *) * INITIAL_FILE_VEC_SIZE);
  f->size = 0;
  f->methods = checked_malloc(sizeof(struct file_vector_methods));
  f->methods->size = file_vector_size;
  f->methods->empty = file_vector_empty;
  f->methods->insert = file_vector_insert;
  f->methods->find = file_vector_find;
}

int
file_vector_size(file_vector_t *f)
{
  return f->size;
}

int
file_vector_empty(file_vector_t *f)
{
  return f->size == 0 ? 1 : 0;
}

void
file_vector_insert(file_vector_t *f, char *c)
{
  if (f->size > INITIAL_FILE_VEC_SIZE)
    {
      int multiplier = f->size % INITIAL_FILE_VEC_SIZE;
      f->filenames = checked_realloc(f->filenames, sizeof(char *) *
				     INITIAL_FILE_VEC_SIZE * multiplier);
    }
  f->filenames[(f->size)++] = c;
}


// returns index of where it was found, -1 otherwise.
int
file_vector_find(file_vector_t *f, char *c)
{
  int size = f->size;
  int i;
  for (i = 0; i < size; i++)
    {
      if(strcmp(f->filenames[i], c) == 0)
	return i;
    }
  return -1;
}

#endif
