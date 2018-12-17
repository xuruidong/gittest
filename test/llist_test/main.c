#include <stdio.h>

#include "link_list.h"

#define NAMESIZE 32

int list_op(void *arg)
{
	int tmp = *(int *)arg;
	printf("%d ", tmp);
	
	if (tmp ==5){
		//return LLIST_TRAVEL_BREAK;
	}
	return 0;
}

int list_op_all(void *arg)
{
	int tmp = *(int *)arg;
	printf("%d ", tmp);
	
	
	return 0;
}

int cmp(const void * key, const void * arg)
{
	int tmp = *(int *)arg;
	if (tmp%2 == 0){
		return 0;
	}
	return 1;
}

int main(void)
{
      LLIST *list;
	  
      list = llist_creat(sizeof(int));
	  int i = 0;
	  for (i=0; i<10; i++){

		  llist_insert(list, &i, LLIST_BACKWARD);
	  }
		
		int num = 0;
		num = llist_getnum(list);
		printf("list num=%d\n", num);
		llist_travel(list , list_op);
		printf("\n");


		llist_travel_delete(list, list_op, &num, cmp, NULL);
		printf("\n");
		llist_travel(list , list_op_all);
		printf("\n");
		num = llist_getnum(list);
		printf("list num=%d\n", num);

      llist_destroy(list, NULL);

      return 0;
}
