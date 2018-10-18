#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__


#define LLIST_FORWARD      1
#define LLIST_BACKWARD     2


struct llist_node_st {
      struct llist_node_st *prev;
      struct llist_node_st *next;
      char data[0];
};

typedef struct {
      int size;
      struct llist_node_st head;
} LLIST;

typedef int llist_cmp(const void *, const void *);
typedef void llist_op(void *);
typedef int llist_node_check(void *);
typedef void llist_node_op(void *);


LLIST *llist_creat(int size);

void llist_destroy(LLIST *, llist_node_op *);

int llist_insert(LLIST *, const void *data, int mode);

void llist_delet(LLIST *, const void *key, llist_cmp *, llist_node_op *node_op);

void *llist_find(LLIST *, const void *key, llist_cmp *);

void llist_travel(LLIST *, llist_op *);

void llist_clear(LLIST *ptr, llist_node_op *node_op);

//int llist_fetch(LLIST *, void *data, const void *key, llist_cmp *);

int llist_getnum(LLIST *);

void llist_check(LLIST *ptr, llist_node_check *node_check);


#endif

