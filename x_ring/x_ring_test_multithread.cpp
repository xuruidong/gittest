#include <errno.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
/*
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <pthread.h>

#include <sys/types.h>

*/
#include <iostream>
#include <thread>
#include "x_ring.h"

//--------------  ringbuf  --------------

#define LOG_BUFF_LEN	1024


void do_when_destroy_ring(void *arg)
{
	char *p = (char *)arg;
	free(p);
}

void thread_enqueue(x_ring_t * r)
{
	int ret = 0;
	int i = 0;
	char buf[20] = {0};
	int buf_len = 0;
	while(1){
		buf_len = snprintf(buf, sizeof(buf), "ring %d", i);
		ret = x_ring_sp_enqueue(r, buf, buf_len);
		if(ret != 0){
			printf("failed to enqueue %d,sleep 3\n",i);
			sleep(1);
		}
		else{
			i++;
			usleep(300000);
		}

	}
}

void thread_dequeue(x_ring_t * r)
{
	int ret = 0;
	x_ring_entry_t entry;
	entry.data = (char *)malloc(20);
		entry.size = 20;
		entry.content_length = 0;
	while(1){
		ret = x_ring_sc_dequeue(r, &entry);
		if(ret != 0){
			printf("x_ring_mc_dequeue fail, %d\n", ret);
			sleep(1);
			continue;
		}
		printf("entry.data:%s\n", entry.data);
		sleep(1);
	}
}

int main(void)
{
	int i = 0;
	int len = 0;
	int ret = 0;
	std::cout<<"start..."<<std::endl;
	x_ring_t * r = x_ring_create(5, 20);


	std::thread task01(thread_enqueue, r);
	std::thread task02(thread_dequeue, r);
	task01.join();
	task02.join();

	
	
	//sleep(1);
	//x_ring_print(r);

	
	//x_ring_destroy(r, do_when_destroy_ring);
	x_ring_destroy(r, NULL);

	return 0;
}
