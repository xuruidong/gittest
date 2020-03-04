
#ifndef __X_LOG_H__
#define __X_LOG_H__


#ifdef __cplusplus
extern "C" {
#endif



#define LOG_BUFF_LEN	1024

enum LOG_LEVEL {
	LOG_LEVEL_FATAL,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_TEST
};


typedef void XLOG;

XLOG * x_log_create(const char *log_name, unsigned int ring_size, int cfg_level);

void x_log_destroy(XLOG *xlog);

int x_log_write(XLOG *logger, int level, const char *fmt, ...);

unsigned long long x_log_get_output_count(XLOG *logger);

#ifdef __cplusplus
}
#endif

#endif
