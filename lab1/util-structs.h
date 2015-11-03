#ifndef UTIL_STRUCTS_H
#define UTIL_STRUCTS_H

#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <stdlib.h>

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
init(queue_t *queue)
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

#endif
