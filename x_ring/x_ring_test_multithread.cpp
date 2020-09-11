#include <errno.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
/*
#include <stdarg.h>

#include <sys/stat.h>
#include <syslog.h>
#include <pthread.h>

#include <sys/types.h>

*/
#include <iostream>
#include <thread>
#include "x_ring.h"

/*******************************
 * g++ x_ring_test_multithread.cpp x_ring.c -lpthread -std=c++11 
 *******************************/

using namespace std;
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

#define TRY_NUMBER	1000
#define MAX_SEQ_NUM	5000000
void thread_enqueue_try_best(x_ring_t * r)
{
	int ret = 0;
	int i = 0;
	int tries = 0;
	char buf[20] = {0};
	int buf_len = sizeof(int);
	while(1){
		tries = 0;
		memcpy(buf, &i, sizeof(int));
		while(1){
			ret = x_ring_sp_enqueue(r, buf, buf_len);
			if(ret != 0){
				
				tries ++;
				if(tries >= TRY_NUMBER){
					printf("failed to enqueue %d,sleep 3\n",i);
					usleep(1);
					tries = 0;
				}
				//sleep(1);
				continue;
			}
			else{
				i++;
				break;
			}
		}
		if(i>=MAX_SEQ_NUM){
			break;
		}
	}
	cout<<"thread_enqueue_try_best end"<<endl;
}

void thread_dequeue_try_best(x_ring_t * r)
{
	int ret = 0;
	int exp_val = 0;
	int tmp = 0;
	int tries = 0;
	x_ring_entry_t entry;
	entry.data = (char *)malloc(20);
		entry.size = 20;
		entry.content_length = 0;
	while(1){
		ret = x_ring_sc_dequeue(r, &entry);
		if(ret != 0){
			//printf("==>x_ring_mc_dequeue failed, %d\n", ret);
			tries ++;
			if(tries >= TRY_NUMBER){
				printf("==>x_ring_mc_dequeue failed, %d\n", ret);
				usleep(1);
				tries = 0;
			}
			continue;
		}
		//printf("entry.data:%s\n", entry.data);
		tmp = *(int *)entry.data;
		if(tmp != exp_val){
			printf("exp: %d, get value:%d\n", exp_val, tmp);
			exit(1);
		}
		cout<<"get value: "<<tmp<<endl;
		exp_val++;
		if(exp_val>=MAX_SEQ_NUM){
			break;
		}
		tries = 0;
		//sleep(1);
	}
	cout<<"thread_dequeue_try_best end"<<std::endl;
}

#define TRY_BEST 	1
int main(void)
{
	int i = 0;
	int len = 0;
	int ret = 0;
	std::cout<<"start..."<<std::endl;
#if TRY_BEST
	x_ring_t * r = x_ring_create(512, 20);


	std::thread task01(thread_enqueue_try_best, r);
	std::thread task02(thread_dequeue_try_best, r);
#else
	x_ring_t * r = x_ring_create(5, 20);


	std::thread task01(thread_enqueue, r);
	std::thread task02(thread_dequeue, r);
#endif

	task01.join();
	task02.join();

	
	
	//sleep(1);
	//x_ring_print(r);

	
	//x_ring_destroy(r, do_when_destroy_ring);
	x_ring_destroy(r, NULL);
	cout<<"end"<<endl;

	return 0;
}
