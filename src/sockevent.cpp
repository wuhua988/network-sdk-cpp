/*****************************************************************************
 Name        : sockevent.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include "sockevent.h"
#include "tcpconnection.h"
#include "log.h"

namespace cppnetwork
{

#ifdef __APPLE__

KqueueSockEvent::KqueueSockEvent()
{
	int num = 3;
	while (num--) {
		_event_handle = kqueue();
		if (_event_handle == -1) {
			LOGE("queue_create failed");
			continue;
		} else {
			break;
		}
	}
}

KqueueSockEvent::~KqueueSockEvent()
{
	if (_event_handle > 0) {
		::close(_event_handle);
		_event_handle = -1;
	}
}

bool KqueueSockEvent::add_event(TcpConnection *conn, bool enable_read, bool enable_write)
{
	return set_event(conn, true, true);
}

bool KqueueSockEvent::set_event(TcpConnection *conn, bool enable_read, bool enable_write)
{
	struct kevent ke;

	unsigned short flags = enable_read ? EV_ENABLE : EV_DISABLE;
	EV_SET(&ke, conn->getfd(), EVFILT_READ, EV_ADD | flags, 0, 0, (void* )conn);

	if (kevent(_event_handle, &ke, 1, NULL, 0, NULL) != 0) {
		LOGE("kevent SET fail handle:%d fd:%d", conn->gethandle(), conn->getfd());
		return false;
	}

	flags = enable_write ? EV_ENABLE : EV_DISABLE;
	memset(&ke, 0, sizeof(ke));
	EV_SET(&ke, conn->getfd(), EVFILT_WRITE, EV_ADD | flags, 0, 0, (void* )conn);
	if (kevent(_event_handle, &ke, 1, NULL, 0, NULL) != 0) {
		LOGW("kevent set_event fail handle:%d fd:%d\n", conn->gethandle(), conn->getfd());
		return false;
	}

	return true;
}

bool KqueueSockEvent::remove_event(TcpConnection *conn)
{
	// EV_DELETE是把所有的事件都刪除了,  所以不关心filter的状态；
	struct kevent ke;

	EV_SET(&ke, conn->getfd(), EVFILT_READ, EV_DELETE, 0, 0, conn);
	if (kevent(_event_handle, &ke, 1, NULL, 0, NULL) != 0) {
		LOGW("kevent remove_event read fail handle:%d fd:%d\n", conn->gethandle(), conn->getfd());
	}

	EV_SET(&ke, conn->getfd(), EVFILT_WRITE, EV_DELETE, 0, 0, conn);
	if (kevent(_event_handle, &ke, 1, NULL, 0, NULL) != 0) {
		LOGW("kevent remove_event write fail handle:%d fd:%d\n", conn->gethandle(), conn->getfd());
	}

	return true;
}

//FOR TEST
void show_kevent(struct kevent &e)
{
	printf("indent = %d, filter = %d flags = %d fflags = %d data = %d udata = %p \n", (int) e.ident, (int) e.filter,
			(int) e.flags, (int) e.fflags, (int) e.data, e.udata);
}

int KqueueSockEvent::get_events(int timeout, IOEvent *out, int cnt)
{
	struct kevent events[128];
	memset(&events, 0, sizeof(events));

	struct timespec t;
	//最小的时间间隔不能小于1ms
	timeout = timeout < 1000 ? 1000 : timeout;
	t.tv_sec = timeout / 1000;
	t.tv_nsec = timeout % 1000;

	int ret = kevent(_event_handle, NULL, 0, events, 1024, &t);

	if (ret < 0) {
		LOGE("KqueueSockEvent::get_events < 0");
		return 0;
	}

	int i = 0;

	for (i = 0; i < ret && i < cnt; i++) {
//		show_kevent(events[i]);

		out[i].conn = (TcpConnection*) events[i].udata;

		if (events[i].filter == EVFILT_READ) {
			out[i]._read_ocurr = true;
		}

		if (events[i].filter == EVFILT_WRITE) {
			out[i]._write_ocurr = true;
		}

	}

	return i;
}

#else

EpollSockEvent::EpollSockEvent()
{
	//todo:尝试3次；MAX_EPOLL_EVENT 128
	int num = 3;
	while (num--)
	{
		_event_handle = epoll_create(MAX_EPOLL_EVENT);
		if(_event_handle < 0)
		{
			LOGE("epoll_create fail:%s", strerror(errno));
			continue;
		}
		else
		{
			break;
		}
	}
}

EpollSockEvent::~EpollSockEvent()
{
	if(_event_handle > 0)
	{
		::close(_event_handle);
		_event_handle = -1;
	}
}

bool EpollSockEvent::add_event(TcpConnection *conn, bool enable_read, bool enable_write)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.ptr = conn;
	// 设置要处理的事件类型
	ev.events = 0;

	if (enable_read)
	{
		ev.events |= EPOLLIN;
	}
	if (enable_write)
	{
		ev.events |= EPOLLOUT;
	}

	//_mutex.lock();
	bool rc = (epoll_ctl(_event_handle, EPOLL_CTL_ADD, conn->getfd(), &ev) == 0);
	//_mutex.unlock();
	return rc;
}

bool EpollSockEvent::set_event(TcpConnection *conn, bool enable_read, bool enable_write)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));

	ev.data.ptr = conn;
	// 设置要处理的事件类型
	ev.events = 0;

	if (enable_read)
	{
		ev.events |= EPOLLIN;
	}
	if (enable_write)
	{
		ev.events |= EPOLLOUT;
	}

	//_mutex.lock();
	bool rc = (epoll_ctl(_event_handle, EPOLL_CTL_MOD, conn->getfd(), &ev) == 0);

	return rc;
}

bool EpollSockEvent::remove_event(TcpConnection *conn)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));

	ev.data.ptr = conn;
	// 设置要处理的事件类型
	ev.events = 0;
	//_mutex.lock();
	bool rc = (epoll_ctl(_event_handle, EPOLL_CTL_DEL, conn->getfd(), &ev) == 0);

	return rc;
}

/****************************
 EPOLLHUP事件触发：当socket的一端认为对方发来了一个不存在的4元组请求的时候,会回复一个RST响应,在epoll上会响应为EPOLLHUP事件
 [1] 当客户端向一个没有在listen的服务器端口发送的connect的时候 服务器会返回一个RST 因为服务器根本不知道这个4元组的存在.
 [2] 当已经建立好连接的一对客户端和服务器,客户端突然操作系统崩溃,或者拔掉电源导致操作系统重新启动(kill pid或者正常关机不行的,因为操作系统会发送FIN给对方).
 这时服务器在原有的4元组上发送数据,会收到客户端返回的RST,因为客户端根本不知道之前这个4元组的存在.
 ****************************/
int EpollSockEvent::get_events(int timeout, IOEvent *ioevents, int cnt)
{
	struct epoll_event events[MAX_EPOLL_EVENT];

	if (cnt > MAX_EPOLL_EVENT)
	{
		cnt = MAX_EPOLL_EVENT;
	}

	int res = epoll_wait(_event_handle, events, cnt, timeout);

	// 初始化
	if (res > 0)
	{
		memset(ioevents, 0, sizeof(IOEvent) * res);
	}

	// 把events的事件转化成IOEvent的事件
	for (int i = 0; i < res; i++)
	{
		ioevents[i].conn = (TcpConnection*) events[i].data.ptr;

		if (events[i].events & (EPOLLERR | EPOLLHUP))
		{
			ioevents[i]._read_ocurr= true;
		}
		if ((events[i].events & EPOLLIN) != 0)
		{
			ioevents[i]._read_ocurr = true;
		}
		if ((events[i].events & EPOLLOUT) != 0)
		{
			ioevents[i]._write_ocurr = true;
		}
	}

	return res;
}

#endif

}
/* namespace cppnetwork */
