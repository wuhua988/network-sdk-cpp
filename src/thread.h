/*****************************************************************************
 Name        : thread.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_THREAD_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_THREAD_H_

#include <list>
#include <pthread.h>
#include <cstring>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "tools.h"

namespace cppnetwork
{

class Runnable
{
public:
	virtual ~Runnable()
	{
	}

	virtual void run(void *param) = 0;
};

class TBThread;

class TBRunnable
{

public:
	virtual ~TBRunnable()
	{
	}
	virtual void run(TBThread *thread, void *arg) = 0;
};

//对linux线程的简单封装，用在非线程池的情况下
class TBThread
{

public:

	TBThread()
	{
		tid = 0;
	}

	bool start(TBRunnable *r, void *a)
	{
		runnable = r;
		args = a;
		return 0 == pthread_create(&tid, NULL, TBThread::hook, this);
	}

	void join()
	{
		if (tid) {
			pthread_join(tid, NULL);
			tid = 0;
		}
	}

	TBRunnable *getRunnable()
	{
		return runnable;
	}

	/**
	 * 得到回调参数
	 *
	 * @return args
	 */
	void *getArgs()
	{
		return args;
	}

	/**
	 * 线程的回调函数
	 *
	 */
	static void *hook(void *arg)
	{
		TBThread *thread = (TBThread*) arg;
		thread->tid = gettid();

		if (thread->getRunnable()) {
			thread->getRunnable()->run(thread, thread->getArgs());
		}

		return (void*) NULL;
	}

private:

	static pthread_t gettid()
	{
		return pthread_self();
	}

private:
	pthread_t tid;      // pthread_self() id
	TBRunnable *runnable;
	void *args;
};

class Thread: public Ref
{

	// POSIX Thread scheduler policies
	enum POLICY
	{
		OTHER, FIFO, ROUND_ROBIN
	};

	// POSIX Thread scheduler relative priorities,
	//
	// Absolute priority is determined by scheduler policy and OS. This
	// enumeration specifies relative priorities such that one can specify a
	// priority withing a giving scheduler policy without knowing the absolute
	// value of the priority.
	enum PRIORITY
	{
		LOWEST = 0, LOWER = 1, LOW = 2, NORMAL = 3, HIGH = 4, HIGHER = 5, HIGHEST = 6, INCREMENT = 7, DECREMENT = 8
	};

	enum STATE
	{
		uninitialized, starting, started, stopping, stopped
	};

	static const int MB = 1024 * 1024;

public:
	Thread(Runnable *runner, void *param = NULL, int policy = FIFO, int priority = NORMAL, int stack_size = 2,
			bool detached = false);

	virtual ~Thread();

	/**
	 * Starts the thread. Does platform specific thread creation and
	 * configuration then invokes the run method of the Runnable object bound
	 * to this thread.
	 */
	virtual void start(void);

	/**
	 * Join this thread. Current thread blocks until this target thread
	 * completes.
	 */
	virtual void join();

	/**
	 * Gets the thread's platform-specific ID
	 */
	virtual pthread_t id()
	{
		return _pthread;
	}

public:
	static void * ThreadMain(void *param);

	Runnable * runable(void)
	{
		return _runner;
	}

private:
	Runnable *_runner;

	pthread_t _pthread;

	void * _param;

	STATE _state;

	/**
	 * POSIX Thread scheduler policies
	 */
	int _policy;

	int _priority;

	int _stackSize;

	bool _detached;

	Thread *_selfRef;
};

class ThreadManager
{
public:
	ThreadManager() :
			_thread_state(false)
	{
	}

	virtual ~ThreadManager();

	virtual bool init(unsigned int nthread, void *param, Runnable *runner);

	virtual void start(void);

	virtual void stop(void);

private:
	typedef std::list<Thread*> ThreadList;

	ThreadList _thread_lst;

	bool _thread_state;
};

} /* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_THREAD_H_ */
