/*****************************************************************************
 Name        : network.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#include <errno.h>
#include "network.h"
#include "sockevent.h"
#include "log.h"

namespace cppnetwork
{
NetWork::OnlineUser::~OnlineUser()
{
	Guard g(_mutex);
	ONLINE_USER_ITER iter = _conns.begin();

	for (; iter != _conns.end(); ++iter) {
		if (iter->second != NULL) {
			//析构函数中强制删除
//			delete iter->second;
			iter->second->release();
		}
	}

	while (!_recycles.empty()) {
		TcpConnection *conn = _recycles.front();
		_recycles.pop_front();
		//强制删除
//		delete conn;
		conn->release();
	}

}

TcpConnection *NetWork::OnlineUser::getconn(int handle)
{
	Guard g(_mutex);
	TcpConnection *conn = NULL;
	ONLINE_USER_ITER iter = _conns.find(handle);
	if (iter != _conns.end()) {
		conn = iter->second;
		conn->add_ref();
	}

	return conn;
}

bool NetWork::OnlineUser::addconn(TcpConnection *conn)
{
	Guard g(_mutex);

	ONLINE_USER_ITER iter = _conns.find(conn->gethandle());
	if (iter == _conns.end()) {
		_conns.insert(make_pair(conn->gethandle(), conn));
		return true;
	} else {
		delete conn; //todo :l
		LOGE("NetWork::addconn fail, NetWork already have this handle:%d", conn->gethandle());
		return false;
	}
}

bool NetWork::OnlineUser::removeconn(int handle)
{
	bool ret = false;
	Guard g(_mutex);
	ONLINE_USER_ITER iter = _conns.find(handle);
	if (iter != _conns.end()) {
		ret = true;
		_recycles.push_back(iter->second);
		_conns.erase(handle);
	}

	return ret;
}

//暂不支持timeout清理工作
void NetWork::OnlineUser::check_timeout(std::map<int, int> &timeout_list)
{
	TcpConnection *conn = NULL;
	time_t now = time(0);

	Guard g(_mutex);
	ONLINE_USER_ITER iter = _conns.begin();
	list<int> timeout_conns;
	for (; iter != _conns.end(); ++iter) {
		conn = iter->second;
		conn->add_ref();

		//连接超时检测
		if (conn->getstate() == CONNECTING || conn->getstate() == CRYPTREGING) {
			//连接超时时间是5s
			if (now - conn->get_last_active_time() > 5) {
				LOGW("handle :%d connect timeout", conn->gethandle());
				//!!!注意：close不能再此处调用！否则与外层的_conns循环混到一起了
				//close(conn->gethandle());
				//onClose(conn->gethandle(), SOCKET_ERROR_TIMEOUT);
				conn->set_last_error(SOCKET_ERROR_CONN_TIMEOUT);
				timeout_list.insert(make_pair((int) conn->gethandle(), conn->get_last_error()));
			}
		} else if (conn->getstate() == CONNECTED) {
			//10分钟没有任何响应就强制回收吧
			if (now - conn->get_last_active_time() > 60 * 10) {
				LOGW("handle :%d timeout", conn->gethandle());
				conn->set_last_error(SOCKET_ERROR_TIMEOUT);
				timeout_list.insert(make_pair((int) conn->gethandle(), conn->get_last_error()));
			}
		}

		conn->release();
	}

	return;
}

void NetWork::OnlineUser::clear()
{
	Guard g(_mutex);
	//将closing的数据清除掉
	while (!_recycles.empty()) {
		TcpConnection *conn = _recycles.front();
		_recycles.pop_front();
		conn->release();
	}
}

NetWork::NetWork()
{
	// TODO Auto-generated constructor stub
	_stop = false;
	_test = false;

#ifdef __APPLE__
	_sock_event = new KqueueSockEvent();
#else
	_sock_event = new EpollSockEvent();
#endif
}

NetWork::~NetWork()
{
	//将启动的线程终止
	_stop = true;
	_threadmgr.stop();

	if (_sock_event != NULL) {
		delete _sock_event;
		_sock_event = NULL;
	}
}

//初始化
void NetWork::init()
{
	_threadmgr.init(1, NULL, this);
	_threadmgr.start();
}

void NetWork::set_test(bool on)
{
	_test = on;
}

bool NetWork::get_test()
{
	return _test;
}

void NetWork::set_test_msg(const std::string &test_msg)
{
	_test_msg = test_msg;
}

std::string &NetWork::get_test_msg()
{
	return _test_msg;
}

int NetWork::connect(const char *ip, unsigned short port, bool encrypt)
{
	int handle = -1;
	TcpConnection *conn = new TcpConnection(this);

//	if(encrypt) conn->set_encrypt();
	if (conn->connect(ip, port)) {
		if (_online_user.addconn(conn)) {
			_sock_event->add_event(conn, true, true);
			handle = conn->gethandle();
		}
	} else {
		LOGI("NetWork::connect connect fail : %s", strerror(errno));
		delete conn;
	}

	return handle;
}

//数据发送，只是投递到发送缓冲区，return 0：表示投递成功
bool NetWork::send(int handle, const char *buffer, int len)
{
	bool ret = false;
	TcpConnection *conn = _online_user.getconn(handle);
	if (conn != NULL) {
		ret = conn->deliver_data(buffer, len);
		conn->release();
	}

	return ret;
}

//主动关闭连接
void NetWork::close(int handle)
{
	TRACE(__PRETTY_FUNCTION__);

	TcpConnection *conn = _online_user.getconn(handle);
	if (conn != NULL) {
		//从kqueue/epoll中线拿出来，防止再触发事件，导致各种复杂耦合
		_sock_event->remove_event(conn);
		//判断removeconn的返回值，主线程和业务线程同时调用close时，产生conn->close重复调用
		if (_online_user.removeconn(handle)) {
			conn->close();
		}
		conn->release();
	}

	return;
}

int NetWork::getStatus(int handle)
{
	TRACE(__PRETTY_FUNCTION__);

	int status = NET_STATE_INVALID_HANDLE;
	TcpConnection *conn = _online_user.getconn(handle);
	if (conn != NULL) {
		switch (conn->getstate()) {
		case WAIT_CLOSE:

		case CLOSED:
			status = NET_STATE_CLOSED;
			break;
			//connecting 和 CRYPTREGING都认为是正在连接状态
		case CONNECTING:
		case CRYPTREGING:
			status = NET_STATE_CONNECTING;
			break;
		case CONNECTED:
			status = NET_STATE_CONNECTED;
			break;
		default:
			break;
		}
		conn->release();
	}

	return status;
}

//连接结果通知
void NetWork::onConnect(int handle)
{
//	printf("NetWork onConnect handle:%d \n", handle);
}

//数据到来通知，交付出来的已经是一个完整的PDU包
void NetWork::onRead(int handle, const char* buffer, int len)
{
//	printf("NetWork onRead handle:%d len:%d buffer:%s\n", handle, len, buffer + 4);
}

//网络断开数据通知，reason：失败处理的缘由
void NetWork::onClose(int handle, int reason)
{
//	printf("NetWork onClose handle:%d reason:%d \n", handle, reason);
}

void NetWork::run(void *param)
{
	TRACE(__PRETTY_FUNCTION__);

	(void) (param);

	this->event_loop();
}

void NetWork::event_loop()
{
	TRACE(__PRETTY_FUNCTION__);

	//最大时间间隔是1s
	int timeout = 1000;
	IOEvent events[128];
	while (!_stop) {
		memset(events, 0, sizeof(events));
		int n = _sock_event->get_events(timeout, events, 128);
		for (int i = 0; i < n; i++) {
			IOEvent &ev = events[i];
			TcpConnection *conn = ev.conn;
			conn->add_ref();
			if (ev._read_ocurr)
				conn->on_read_event();
			if (ev._write_ocurr) {
				conn->on_write_event();
			}
			conn->release();
		}
		//定时处理
		time_process(time(NULL));
	}

}

void NetWork::time_process(time_t now)
{
	static time_t last_time = time(0);

	//如果还是在1s时间内直接返回
	if (now - last_time < 1)
		return;

	TRACE(__PRETTY_FUNCTION__);

	last_time = now;

	//处理超时的连接
	std::map<int, int> timeout_list;
	_online_user.check_timeout(timeout_list);
	std::map<int, int>::iterator it = timeout_list.begin();
	for (; it != timeout_list.end(); ++it) {
		this->close(it->first);
		onClose(it->first, it->second);
	}

	//垃圾清理
	_online_user.clear();

	return;
}

#define MAX_INT ((unsigned)(-1)>>1)
int NetWork::make_handle()
{
	static int h = 0;

	TcpConnection *conn = NULL;

	h++;
	while ((conn = _online_user.getconn(h)) != NULL) {
		h++;
		if (h >= MAX_INT) {
			h = 0;
		}
		conn->release();
	}

	return h;
}

} /* namespace cppnetwork */
