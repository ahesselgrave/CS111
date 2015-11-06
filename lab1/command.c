#include "alloc.h"
#include "command.h"
#include <stdlib.h>

int
command_stream_size(command_stream_t c)
{
  return c->numNodes;
}

void
command_stream_insert(command_stream_t c, command_t command)
{
  c->numNodes++;
  c->tail->command = command;
  c->tail->next = checked_malloc(sizeof(struct command_node));
  c->tail = c->tail->next;
  c->tail->next = NULL;
}

void
command_stream_init(command_stream_t c)
{
  c->head = checked_malloc(sizeof(struct command_node));
  c->head->next = NULL;
  c->tail = c->head;
  c->numNodes = 0;
  c->methods = checked_malloc(sizeof(struct command_stream_methods));
  c->methods->size = command_stream_size;
  c->methods->insert = command_stream_insert;
}
