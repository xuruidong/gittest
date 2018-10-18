#include <stdlib.h>
#include <string.h>
#include "link_list.h"

LLIST *llist_creat(int size)
{
      LLIST *new_node;

      new_node = (LLIST *)malloc(sizeof(*new_node));
      if (new_node == NULL) {
	    return NULL;
      }

      new_node->size = size;
      new_node->head.prev = new_node->head.next = &new_node->head;

      return new_node;
}

void llist_destroy(LLIST *ptr, llist_node_op *node_op)
{
    struct llist_node_st *cur, *save;
	if (!ptr)
		return;
	
      for (cur = ptr->head.next; cur != &ptr->head; cur = save) {
		save = cur->next;
		if (node_op)
			node_op(cur->data);
		free(cur);
      }
      free(ptr);
}

void llist_clear(LLIST *ptr, llist_node_op *node_op)
{
	struct llist_node_st *cur, *save;

	for (cur = ptr->head.next; cur != &ptr->head; cur = save) {
		save = cur->next;
		if (node_op)
			node_op(cur->data);
		free(cur);
	}
	ptr->head.prev = ptr->head.next = cur;
}

int llist_insert(LLIST *ptr, const void *data, int mode)
{
      struct llist_node_st *newnode;

      newnode = (struct llist_node_st *)malloc(sizeof(*newnode) + ptr->size);
      if (newnode == NULL) {
	    return -1;
      }
      memcpy(newnode->data, data, ptr->size);

      if (mode == LLIST_FORWARD) {
	    newnode->next = ptr->head.next;
	    newnode->prev = &ptr->head;
      } else if (mode == LLIST_BACKWARD) {
	    newnode->next = &ptr->head;
	    newnode->prev = ptr->head.prev;
      }

      newnode->next->prev = newnode;
      newnode->prev->next = newnode;

      return 0;
}

static struct llist_node_st *find__(LLIST *ptr, const void *key, llist_cmp *cmp)
{
      struct llist_node_st *cur;

      for (cur = ptr->head.next; cur != &ptr->head; cur = cur->next) {
	    if (!cmp(key, cur->data)) {
		  break;
	    }
      }

      return cur;
}

void llist_delet(LLIST *ptr, const void *key, llist_cmp *cmp, llist_node_op *node_op)
{
	struct llist_node_st *node;

	node = find__(ptr, key, cmp);
	if (node == &ptr->head) {
	    return;
	}

	node->next->prev = node->prev;
	node->prev->next = node->next;
	
	if (node_op)
		node_op(node->data);
	
	free(node);
}

void *llist_find(LLIST *ptr, const void *key, llist_cmp *cmp)
{
      struct llist_node_st *node;

      node = find__(ptr, key, cmp);
      if (node == &ptr->head) {
	    return NULL;
      }

      return node->data;

}
void llist_travel(LLIST *ptr, llist_op *op)
{
	struct llist_node_st *cur;
	
	for (cur = ptr->head.next; cur != &ptr->head; cur = cur->next) {
		op(cur->data);
	}
}

int llist_getnum(LLIST *ptr)
{
      int i;
      struct llist_node_st *cur;

      for (i = 0, cur = ptr->head.next; \
	   cur != &ptr->head; \
	   cur = cur->next, i++)
	    ;

      return i;
}

void llist_check(LLIST *ptr, llist_node_check *node_check)
{
    struct llist_node_st *cur, *save;
	if (!ptr)
		return;
	
      for (cur = ptr->head.next; cur != &ptr->head; cur = save) {
		save = cur->next;
		if (node_check){
			if(node_check(cur->data) == 0){
				cur->next->prev = cur->prev;
      			cur->prev->next = cur->next;
				free(cur);
			}	
		}
      }
      //free(ptr);
}


