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
#include "x_ring.h"

//--------------  ringbuf  --------------

#define LOG_BUFF_LEN	1024


void do_when_destroy_ring(void *arg)
{
	char *p = (char *)arg;
	free(p);
}


int main(void)
{
	int i = 0;
	char buf[LOG_BUFF_LEN] = {0};
	int len = 0;
	int ret = 0;
	x_ring_t * r = x_ring_create(3);


	for (i=0; i< 20; i++){
		len = snprintf(buf, LOG_BUFF_LEN, "1234567890---%d\n", i);
		ret = x_ring_sp_enqueue(r, buf, len);
		if(ret != 0){
			printf("log_ringbuf_enqueue fail, %d\n",i);
			break;
		}
		printf("set %s\n", buf);
		usleep(1000);
	}
	
	
	//sleep(1);
	//x_ring_print(r);

	x_ring_entry_t entry[4];
	for(i=0; i<sizeof(entry)/sizeof(x_ring_entry_t); i++){
		
		entry[i].data = malloc(200);
		entry[i].size = 200;
		entry[i].content_length = 0;
		ret = x_ring_sc_dequeue(r, &entry[i]);
		if(ret != 0){
			printf("x_ring_mc_dequeue fail, %d, %d\n",i, ret);
			break;
		}
		printf("out===%s\n", entry[i].data);
	}
	
	//x_ring_destroy(r, do_when_destroy_ring);
	x_ring_destroy(r, NULL);

	return 0;
}
