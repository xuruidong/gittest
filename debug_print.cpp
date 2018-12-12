#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include "debug_print.h"

#define DEBUG_WITH_PRINTF	1

static int debug_level = DEBUG_LEVEL_DEBUG;


int dbg_level_set(int level)
{
	if(level <= MAX_DEBUG_LEVEL && level >=MIN_DEBUG_LEVEL)
	{
		debug_level = level;
		return 0;
	}
	return -1;
}

void dbg_print(int level, const char *fmt,...)
{
	if(level>debug_level)
		return;
	
	va_list ap;
	char buf[2048];
	va_start (ap, fmt);
	vsnprintf (buf,2048, fmt, ap);
	buf[2047]=0;
	va_end(ap);
	syslog(LOG_INFO|LOG_LOCAL5,"%s",buf);
#if DEBUG_WITH_PRINTF
	printf("%s",buf);
#endif
}

const char * xstrerror(void)
{
	static char xstrerror_buf[128];
	const char *errmsg;

	errmsg = strerror(errno);
	if (!errmsg || !*errmsg)
		errmsg = "Unknown error";

	snprintf(xstrerror_buf, 128, "(%d) %s", errno, errmsg);
	return xstrerror_buf;
}


