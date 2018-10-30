#include <stdio.h>

//#include "stack.h"
#include "queue.h"

#define NAMESIZE 32

struct score {
      int id;
      char name[NAMESIZE];
      int math;
};

void print_s(void *data)
{
      struct score *d = data;
      printf("%d %s %d\n", d->id, d->name, d->math);
}

int id_cmp(const void *key, const void *record)
{
      const int *id = key;
      const struct score *r = record;

      return *id - r->id;
}

int main(void)
{
    int tmp;
    int i;
    int ret;
/*    //struct score tmp;
    int tmp;
	STACK *list;
    int ret;
    int i;

    list = stack_creat(sizeof(int));
    for (i = 7; i > 0; i--) {
		tmp = i;
	    //tmp.math = 100 - i;
	    //snprintf(tmp.name, NAMESIZE, "stu%d", i);
	    stack_push(list, &tmp);
    }

      for (i = 7; i > 0; i--) {
	    ret = stack_pop(list, &tmp);
		  if (ret == -1) {
		  printf("stack empty.\n");
		  break;
	    }
		printf("%d\n",tmp);
      }

      stack_destroy(list);
    
 */   
   	QUEUE *queue;

    queue = queue_creat(sizeof(int));
    for (i = 7; i > 0; i--) {
        tmp = i;
        queue_enq(queue,&tmp);
    }
    
    for (i = 7; i > 0; i--) {
        ret = queue_deq(queue,&tmp);
        if (ret == -1) {
            printf("stack empty.\n");
            break;
        }
        printf("%d\n",tmp);
        //print_s(&tmp);
    }
    
    queue_destroy(queue);
    
    return 0;
}
