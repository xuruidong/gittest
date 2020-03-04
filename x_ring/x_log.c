
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
/*
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include <sys/file.h>

*/
#include <pthread.h>
#include <unistd.h>

#include "x_ring.h"
#include "x_log.h"

static char log_level_str[][32] = {
	"Fatal",
	"Error",
	"Warning",
	"Info",
	"Debug",
	"Test"
};


enum{
	LOG_THR_STATUS_INIT = 0,
	LOG_THR_STATUS_RUNNING,
	LOG_THR_STATUS_STOPPING,
	LOG_THR_STATUS_STOPPED
};

/*
static int g_app_log_thread_status = LOG_THR_STATUS_INIT;
static struct app_log_t g_app_log;


static int app_log_rename(void);
static void *app_log_file_size_check(void *p);
static int app_log_check_thread_sart(void);
*/



//-------  log ---------------------

typedef struct x_log_s {
	
	FILE *fp;
	x_ring_t *log_ring;
	unsigned long long log_count;
	int level;
	pthread_t tid;
	int thread_status;

} x_log_t;

static void * x_log_thread(void *args)
{
	int ret = 0;
	x_log_t *xlogger  = (x_log_t *)args; 
	//char rbuf[1024] = {0};
	x_ring_entry_t ring_entry;
	ring_entry.size = LOG_BUFF_LEN;
	ring_entry.content_length = 0;
	ring_entry.data = malloc(ring_entry.size);
	
	while(1){
		ret = x_ring_sc_dequeue(xlogger->log_ring, &ring_entry);
		if( ret != X_RING_RET_SUCCESS){
			printf("[%s:%d] no log\n", __FUNCTION__, __LINE__);
			usleep(1);
			continue;
		}
		//printf("%s\n", rbuf);
		fwrite(ring_entry.data, 1, ring_entry.content_length, xlogger->fp);
		//fflush(xlogger->fp);
		xlogger->log_count ++;
	}

	free(ring_entry.data);
	return NULL;
}

XLOG * x_log_create(const char *log_name, unsigned int ring_size, int cfg_level)
{
	x_log_t *logserver;
	
	logserver = (x_log_t *)malloc(sizeof(x_log_t));
	
	if (log_name[strlen(log_name)-1] == '/')
	{
		printf("%s: Log file error, \"%s\" is not a file\n", log_level_str[LOG_LEVEL_FATAL], log_name);
		return NULL;
	}

	logserver->fp = fopen(log_name, "a+");
	if (NULL == logserver->fp)
	{
		printf("%s: Log open file \"%s\" fail!\n", log_level_str[LOG_LEVEL_FATAL], log_name);
		perror("fopen");
		return NULL;
	}

	logserver->log_ring = x_ring_create(ring_size);
	if (NULL == logserver->log_ring){
		printf("error: x_log_create: x_ring_create\n");
		fclose(logserver->fp);
		free(logserver);
		return NULL;
	}

	logserver->level = cfg_level;
	logserver->thread_status = LOG_THR_STATUS_INIT;
	if(pthread_create(&(logserver->tid), NULL, x_log_thread, logserver) < 0){

		printf("pthread_create\n");
		x_ring_destroy(logserver->log_ring, NULL);
		fclose(logserver->fp);
		free(logserver);
		return NULL;
	}
	logserver->thread_status = LOG_THR_STATUS_RUNNING;

	return logserver;
}

void x_log_destroy(XLOG *logger)
{
	x_log_t *xlog = (x_log_t *)logger;

	xlog->thread_status = LOG_THR_STATUS_STOPPING;
	pthread_join(xlog->tid, NULL);

	if(xlog->log_ring){
		x_ring_destroy(xlog->log_ring, NULL);
		xlog->log_ring = NULL;
	}
	
	if(xlog->fp){
		fclose(xlog->fp);
		xlog->fp = NULL;
	}
	
}

int x_log_write(XLOG *logger, int level, const char *fmt, ...)
{
	x_log_t *xlog = (x_log_t *)logger;
	char sbuf[LOG_BUFF_LEN] = {0};
	if ((unsigned int)level > xlog->level){
		return -1;
	}


	va_list args;
	time_t    current_time = time(NULL);
	struct tm now;
	localtime_r( &current_time, &now );

	int ret = 0;
	ret = snprintf(sbuf, LOG_BUFF_LEN, "%04d-%02d-%02d %02d:%02d:%02d: %s ", 1900+now.tm_year, now.tm_mon+1, now.tm_mday,now.tm_hour,now.tm_min, now.tm_sec, log_level_str[level]);
	va_start(args, fmt);
	ret += vsnprintf(sbuf+ret, LOG_BUFF_LEN-ret, fmt, args);
	va_end(args);

	while(x_ring_mp_enqueue(xlog->log_ring, sbuf, ret) != 0){
		usleep(1);
		//printf("log_ringbuf_enqueue fail, \n");
		//return -2;
	}
	
	return 0;
}

unsigned long long x_log_get_output_count(XLOG *logger)
{
	return ((x_log_t *)logger)->log_count;
}

//-----------------=========================----------------
#if 0
XLOG * g_logger;

int app_log_init(const char *filename, int size, int save_num, int create_period)
{
	g_logger = x_log_create(filename, 5000, LOG_LEVEL_TEST);

}

int app_log_write(int level, int cfg_level, const char *fmt, ...)
{
	int ret = 0;
	ret = x_log_write(g_logger, LOG_LEVEL_TEST, "nihao1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ1234567890 %d\n", cfg_level);
	if(ret != 0){
		printf("log error\n");
	}
	return 0;
}
#endif
/*
int main(void)
{
	int i = 0;
	char buf[LOG_BUFF_LEN] = {0};
	int len = 0;
	int ret = 0;
	log_ringbuf_t * r = ringbuf_create(10);


	for (i=0; i< 11; i++){
		len = snprintf(buf, LOG_BUFF_LEN, "1234567890---%d\n", i);
		ret = log_ringbuf_enqueue(r, buf, len);
		if(ret != 0){
			printf("log_ringbuf_enqueue fail, %d\n",i);
			break;
		}
		printf("set %s\n", buf);
		usleep(1000);
	}
	

	//sleep(1);
	log_ringbuf_print(r);

	return 0;
}
*/