/*****************************************************************************
 Name        : log.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include "log.h"

namespace cppnetwork
{

void LOG::log_print(int loglevel, const char *format, ...)
{
#define LOG_MAX_LEN 128

	const char *flag = NULL;
	switch (loglevel) {
	case LOG_DEBUG:
		flag = "DEBUG";
		break;
	case LOG_INFO:
		flag = "INFO";
		break;
	case LOG_WARN:
		flag = "WARN";
		break;
	case LOG_ERROR:
		flag = "ERROR";
		break;
	default:
		flag = "INFO";
	}

	char buffer[LOG_MAX_LEN] = { 0 };

	va_list ap;
	if (format != NULL) {
		va_start(ap, format);
		vsnprintf(buffer, LOG_MAX_LEN - 1, format, ap);
		va_end(ap);
	}

	printf("%s-%s\n", flag, buffer);
}

} /* namespace cppnetwork */
