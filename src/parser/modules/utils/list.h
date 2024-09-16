#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct node {
  void* data;
  struct node* next;
} Node;

typedef struct list {
  int64_t size;
  Node* head;
} List;


List *ics_get_all_blocks(const  char *buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size);
List *list_create();
void list_append(List *list, void *data);

Node *list_create_node(void *data);

void list_free(List *list, void (*destroy)(void *));

void list_iterate(const List *list, void (*func)(void *, void *), void *extra_args);

#endif
