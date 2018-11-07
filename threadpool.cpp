#include <stdio.h>
#include "threadpool.h"
//#include "debug_print.h"

typedef struct {
	void *(*function)(void *);
	void *arg;
} threadpool_task_t;

static void *thread_func(void *arg)
{
	class threadpool *thp = (class threadpool *)arg;
	thp->work();
	return NULL;
}

threadpool :: threadpool(int threadnum, int queue_max)
{
	int err = 0;
	int i = 0;
	queue_max_size = queue_max;
	queue_size = 0;
	task_queue = queue_creat(sizeof(threadpool_task_t));
	if (!task_queue){


	}

	if (pthread_mutex_init(&lock, NULL) < 0 ||
		//pthread_mutex_init(&task_list_lock, NULL) < 0 ||
		pthread_cond_init(&cond_queue_not_empty, NULL) < 0 ||
		pthread_cond_init(&cond_queue_not_full, NULL) < 0){

		perror("pthread_mutex_init");
		return;
	}
	
	thread_num = threadnum;
	if (thread_num <= 0){
		thread_num = 1;
	}
	threads = new pthread_t[thread_num];
	if (!threads){
		perror("threadpool new");
	}

	for (i=0; i<thread_num; i++){
		//printf("[%s:%d] addr=%p\n ", __FILE__, __LINE__, &(threads[i]));
		err = pthread_create(&(threads[i]), NULL, thread_func, this);
		if (err < 0){
			perror("pthread_create ");
			return;
		}
	}
}

threadpool :: ~threadpool()
{

	delete [] threads;
	threads = NULL;
	pthread_cond_destroy(&cond_queue_not_empty);
    pthread_cond_destroy(&cond_queue_not_full);
	pthread_mutex_destroy(&lock);
	queue_destroy(task_queue);
}

int threadpool :: add_task(void *(*function)(void *arg), void *arg)
{
	int ret = 0;
	threadpool_task_t task;
	pthread_mutex_lock(&lock);
	while(queue_size >= queue_max_size){
		pthread_cond_wait(&cond_queue_not_full, &lock);
	}
	task.function = function;
	task.arg = arg;
	ret = queue_enq(task_queue, &task);
	if (ret < 0){
		printf("error\n");

	}
	queue_size ++;
	pthread_cond_signal(&cond_queue_not_empty);
   	pthread_mutex_unlock(&lock);
   	return 0;
}

void *threadpool :: work()
{
	threadpool_task_t task;
	int ret = 0;
	while (1){
		pthread_mutex_lock(&lock);
		while(queue_size <= 0){
			pthread_cond_wait(&cond_queue_not_empty, &lock);
		}
		ret = queue_deq(task_queue, &task);
		if (ret < 0){
			continue;
		}
		queue_size --;

		pthread_cond_broadcast(&cond_queue_not_full);
		pthread_mutex_unlock(&lock);

		(*(task.function))(task.arg);

	}
	pthread_exit(NULL);
}
