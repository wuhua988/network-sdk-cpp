/*****************************************************************************
 Name        : log.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 在android编译是cpp中，#include "log.h"放到最后
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_LOG_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_LOG_H_

namespace cppnetwork
{

#include <stdio.h>
#include <stdarg.h>

#ifdef ANDROID
#include <android/log.h>
#include <jni.h>
#endif

#define LOG_DEBUG  (0)
#define LOG_INFO   (1)
#define LOG_WARN   (2)
#define LOG_ERROR  (3)

class LOG
{
public:
	static void log_print(int loglevel, const char *format, ...);
};

class FunTrace
{
public:
	FunTrace(const char *fun)
	{
		_fun = fun;
		printf("=================INTO===== %s\n", _fun);
	}

	virtual ~FunTrace()
	{
		printf("=================EXIT===== %s\n", _fun);
	}

	const char *_fun;
};

//#define ANDROID
/********** 如果是android，通过自己的日志系统打印 ******************/
#ifdef ANDROID

#ifdef DEBUG
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "libnetwork", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "libnetwork", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "libnetwork", __VA_ARGS__))
#else

#define LOGV(...)
#define LOGD(...)
#define LOGI(...)

#endif

#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "libnetwork", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "libnetwork", __VA_ARGS__))

#ifdef __TRACE
#define TRACE(fmt)     FunTrace a(fmt);
#else
#define TRACE(fmt)
#endif

/***********************************/

#else

#ifdef __DEBUG
#define LOGV(fmt, ...) LOG::log_print( LOG_DEBUG, fmt, ## __VA_ARGS__ )
#define LOGD(fmt, ...) LOG::log_print( LOG_DEBUG, fmt, ## __VA_ARGS__ )
#define LOGI(fmt, ...) LOG::log_print( LOG_INFO,  fmt, ## __VA_ARGS__ )

#else
#define LOGV(fmt, ...)
#define LOGD(fmt, ...)
#define LOGI(fmt, ...)
#define TRACE(fmt)
#endif

#ifdef __TRACE
#define TRACE(fmt)     FunTrace a(fmt);
#else
#define TRACE(fmt)
#endif

#define LOGW(fmt, ...) LOG::log_print(LOG_WARN, fmt, ## __VA_ARGS__)
#define LOGE(fmt, ...) LOG::log_print(LOG_ERROR,fmt, ## __VA_ARGS__)

#endif

} /* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_LOG_H_ */
