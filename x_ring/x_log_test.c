#include <errno.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <pthread.h>


#include "x_log.h"

#define THREAD_NUMS	8
#define LOG_NUMS	100000
void *thread_log(void *arg)
{
	XLOG *logger = (XLOG *)arg;
	int i = 0;
	for (i=0; i<LOG_NUMS; i++){
		x_log_write(logger, LOG_LEVEL_ERROR, "nihao1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ1234567890 %d\n", i);
		//usleep(1);
	}
	return NULL;
}

//#define APP_LOG 1

int main(void)
{
	XLOG * logger = x_log_create("log/app_log.log", 4096, LOG_LEVEL_TEST);
	
	clock_t count = clock();
	clock_t count_end = 0;

	int i = 0;
#if 0
	
	for (i=0; i<LOG_NUMS; i++){
#ifdef APP_LOG
		app_log_write(1, 1, "nihao1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ1234567890 %d", i);
#else
		app_log_write(1, i, "nihao1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ1234567890 %d", i);
#endif
		//usleep(1);
	}
#else
	pthread_t tids[THREAD_NUMS];
	
	for(i=0; i<THREAD_NUMS;i++){
		pthread_create(&tids[i], NULL, thread_log, logger);
	}

	for(i=0; i<THREAD_NUMS;i++){
		pthread_join(tids[i], NULL);
	}

#endif
	count_end = clock();

	printf("clock %Lf\n", (long double)(count_end-count));
	usleep(100000);
	printf("out ount=%llu\n", x_log_get_output_count(logger));

	int c = 0;
	for(i=0; i<THREAD_NUMS; i++){
		//c +=counts[i];
	}
	printf("cou=%d\n",c);

	return 0;
}
