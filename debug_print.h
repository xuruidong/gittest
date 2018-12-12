#ifndef __DBG_PRINT__
#define __DBG_PRINT__

#define DEFAULT_DEBUG_LEVEL (2)
#define MAX_DEBUG_LEVEL 	DEBUG_LEVEL_DEBUG
#define MIN_DEBUG_LEVEL		DEBUG_LEVEL_FATAL

#define DEBUG_LEVEL_FATAL (1)
#define DEBUG_LEVEL_ERROR (2)
#define DEBUG_LEVEL_WARN (3)
#define DEBUG_LEVEL_INFO (4)
#define DEBUG_LEVEL_DEBUG (5)

#define PRINT_LINE()	printf("[%s:%s:%d]\n",__FILE__,__func__,__LINE__)

int dbg_level_set(int level);
void dbg_print(int level, const char *fmt,...);
const char * xstrerror(void);


#endif

