#include "list.h"

List *list_create() {
  List *new_list = (List *)malloc(sizeof(List));
  new_list->size = 0;
  new_list->head = NULL;
  return new_list;
}

Node *list_node_create(void *data){
  Node *node = malloc(sizeof(Node));
  node->data = data;
  node->next = NULL;
  return node;
}


void list_append(List *list, void *data){
  Node *new_node = list_node_create(data); 
  Node *current_node = list->head;

  // nothing is in the list
  if (current_node == NULL){
    list->head = new_node;
    list->size++;
    return;
  }

  while (current_node != NULL){
    if (current_node->next == NULL){
      current_node->next = new_node;
      list->size++;
      return;
    }

    current_node = current_node->next;
  }
}

/* This function will free a List struct.
* The destroy function *must* be the proper destroy function for the data
*/
void list_free(List *list, void (*destroy)(void *)){
  if (list == NULL) return;
  Node *current_node = list->head;
  while (current_node != NULL){
    Node *next_node = current_node->next;
    if (current_node->data != NULL) destroy(current_node->data);
    free(current_node);
    current_node = next_node;
  }

  free(list);
}

void list_iterate(const List *list, void (*func)(void *, void *), void *extra_args){
  if (list == NULL) return;

  Node *current_node = list->head;
  while (current_node != NULL){
    func(current_node->data, extra_args);
    current_node = current_node->next;
  }
}

