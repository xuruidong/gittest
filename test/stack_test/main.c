#include <stdio.h>

#include "stack.h"

#define NAMESIZE 32


int main(void)
{
      STACK *list;
	  int tmp=0;
      int ret;
      int remainder=0;
	  int dec=13;
      list = stack_creat(sizeof(int));
		while(dec)
		{
			remainder = dec%2;
			dec = dec/2;
			stack_push(list,&remainder);
		}
		while(1)
		{
			 ret = stack_pop(list, &tmp);
			 if(-1 == ret)
			 {
				break;
			 }
			 printf("%d",tmp);
		}

      stack_destroy(list);

      return 0;
}
