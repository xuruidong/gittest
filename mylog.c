/*	$OpenBSD: err.c,v 1.2 2002/06/25 15:50:15 mickey Exp $	*/

/*
 * log.c
 *
 * Based on err.c, which was adapted from OpenBSD libc *err* *warn* code.
 *
 * Copyright (c) 2005-2012 Niels Provos and Nick Mathewson
 *
 * Copyright (c) 2000 Dug Song <dugsong@monkey.org>
 *
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifdef _WIN32
#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <errno.h>
 #include "mylog.h"


static mylog_log_cb log_fn = NULL;
static int g_mylog_level = MYLOG_LOG_DEBUG;

void mylog_set_log_callback(mylog_log_cb cb)
{
	log_fn = cb;
}

void mylog_set_log_level(int log_level)
{
	g_mylog_level = log_level;
}

static void mylog_log(int severity, const char *msg)
{
	if (severity < g_mylog_level){
		return;
	}
	if (log_fn)
		log_fn(severity, msg);
	else {
		const char *severity_str;
		switch (severity) {
		case MYLOG_LOG_DEBUG:
			severity_str = "debug";
			break;
		case MYLOG_LOG_MSG:
			severity_str = "msg";
			break;
		case MYLOG_LOG_WARN:
			severity_str = "warn";
			break;
		case MYLOG_LOG_ERR:
			severity_str = "err";
			break;
		default:
			severity_str = "???";
			break;
		}
		(void)fprintf(stderr, "[%s] %s\n", severity_str, msg);
	}
}

void mylog_logv_(int severity, const char *fmt, va_list ap)
{
	char buf[1024];

	if (fmt != NULL)
		vsnprintf(buf, sizeof(buf), fmt, ap);
	else
		buf[0] = '\0';

	mylog_log(severity, buf);
}

void mylog_loging(int severity, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	mylog_logv_(severity, fmt, ap);
	va_end(ap);
}