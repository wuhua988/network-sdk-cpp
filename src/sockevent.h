/*****************************************************************************
 Name        : sockevent.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKEVENT_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKEVENT_H_

namespace cppnetwork
{

class TcpConnection;

struct IOEvent
{
	bool _read_ocurr;
	bool _write_ocurr;
	TcpConnection *conn;
};

class SockEvent
{
public:
	SockEvent() :
			_event_handle(-1)
	{
	}
	;

	virtual ~SockEvent()
	{
	}
	;

	virtual bool add_event(TcpConnection *conn, bool enable_read, bool enable_write) = 0;

	virtual bool set_event(TcpConnection *conn, bool enable_read, bool enable_write) = 0;

	virtual bool remove_event(TcpConnection *conn) = 0;

	virtual int get_events(int timeout, IOEvent *events, int cnt) = 0;

protected:
	int _event_handle;
};

#ifdef __APPLE__

class KqueueSockEvent: public SockEvent
{
public:
	KqueueSockEvent();

	virtual ~KqueueSockEvent();

	bool add_event(TcpConnection *conn, bool enable_read, bool enable_write);

	bool set_event(TcpConnection *conn, bool enable_read, bool enable_write);

	bool remove_event(TcpConnection *conn);

	int get_events(int timeout, IOEvent *events, int cnt);

private:

};

#else

class EpollSockEvent : public SockEvent
{
public:

#define MAX_EPOLL_EVENT (128)

	EpollSockEvent();

	virtual ~EpollSockEvent();

	bool add_event(TcpConnection *conn, bool enable_read, bool enable_write);

	bool set_event(TcpConnection *conn, bool enable_read, bool enable_write);

	bool remove_event(TcpConnection *conn);

	int get_events(int timeout, IOEvent *events, int cnt);
};

#endif

}
/* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKEVENT_H_ */
