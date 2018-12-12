#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

#define NAMESIZE 32

void *job(void *arg)
{
	int i = 0;
	int *p = (int *)arg;
	for (i=0; i< 10; i++){
		printf("job, %d\n", *p);
		sleep(1);
	}
	
	return NULL;
}

int main(void)
{
      threadpool *thp;
      thp = new threadpool(2, 2);
      printf("[%s:%d]\n", __FILE__, __LINE__);
      //sleep(1);
      int i = 1;
      thp->add_task(job, &i);
      printf("[%s:%d]\n", __FILE__, __LINE__);
      int n = 2;
      thp->add_task(job, &n);
      int n2 = 3;
      thp->add_task(job, &n2);
      int n3 = 4;
      thp->add_task(job, &n3);
      int n4 = 5;
      thp->add_task(job, &n4);
      int n5 = 6;
      thp->add_task(job, &n5);
      printf("add end\n");
      sleep(120);
      printf("speep end\n");
      delete thp;
      sleep(3);
      printf("end\n");
      return 0;
}
