#include <stdio.h>

#include "mylog.h"


void print_log(int s, const char *msg)
{

	printf("haha, %s\n", msg);
}

int main(void)
{
    mylog_set_log_level(MYLOG_LOG_ERR); 
    mylog_set_log_callback(print_log); 
	mylog_loging(MYLOG_LOG_DEBUG, "debug---");
	mylog_loging(MYLOG_LOG_MSG, "msg---");
	mylog_loging(MYLOG_LOG_WARN, "warn---");
	mylog_loging(MYLOG_LOG_ERR, "err---");
	//mylog_loging(MYLOG_LOG_DEBUG, "debug---");
      return 0;
}
