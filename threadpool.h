#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>

#include "queue.h"

class threadpool {
private:
	pthread_mutex_t lock;
	pthread_cond_t  cond_queue_not_full;       /* task queue is not full */
   	pthread_cond_t  cond_queue_not_empty;      /* task queue is not empty */

	pthread_t *threads;
	int thread_num;

	QUEUE *task_queue;
	int queue_max_size;
	int queue_size;

	
	
	
public:
	int add_task(void *(*function)(void *arg), void *arg);
	void * work(void);
	threadpool(int threadnum, int queue_max);
	~threadpool();
	/*
	int mutex_lock(){
		pthread_mutex_lock(&lock);
	}
	int mutex_unlock(){
		erturn pthread_mutex_unlock(&lock);
	}
	int cond_queue_not_empty_wait(){
		return pthread_cond_wait(&cond_queue_not_empty, &lock);
	}
	int queue_size_decrease(){
		return --queue_size;
	}
	int cond_queue_not_full_broadcast(){
		return pthread_cond_broadcast(&cond_queue_not_full);
	}
	int get_queue_size(){
		return queue_size;
	}
	*/
};

#endif

