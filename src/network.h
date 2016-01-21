/*****************************************************************************
 Name        : network.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_NETWORK_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_NETWORK_H_

#include "interface.h"
#include "thread.h"
#include "mutex.h"
#include "sockevent.h"
#include "tcpconnection.h"
#include <map>

namespace cppnetwork
{

class NetWork: public Runnable
{
public:
	typedef std::map<int, TcpConnection*> ONLINE_USER;
	typedef std::map<int, TcpConnection*>::iterator ONLINE_USER_ITER;
	typedef std::list<TcpConnection*> CLOSING_USER;
	typedef std::list<TcpConnection*> CLOSING_USER_ITER;

	friend class TcpConnection;

	//在线使用
	class OnlineUser
	{
	public:
		OnlineUser()
		{
		}

		virtual ~OnlineUser();

		TcpConnection *getconn(int handle);

		bool addconn(TcpConnection *conn);

		bool removeconn(int handle);

		void check_timeout(std::map<int, int> &timeout_list);

		void clear();

	private:
		//一般情况就一个连接，_conns和_recycles用一把锁;
		Mutex _mutex;
		ONLINE_USER _conns;
		CLOSING_USER _recycles;
	};

public:
	NetWork();

	virtual ~NetWork();

	//初始化
	void init();

	//返回一个网络操作句柄handler,
	//如果net_handler为NULL,默认调用IOSNetWork this回调
	int connect(const char *ip, unsigned short port, bool encrypt = false);

	//数据发送，只是投递到发送缓冲区，return 0：表示投递成功
	bool send(int handle, const char *buffer, int len);

	//主动关闭连接
	void close(int handle);

	//获取handle的连接状态
	int getStatus(int handle);

	//连接结果通知
	virtual void onConnect(int handle);

	//数据到来通知，交付出来的已经是一个完整的PDU包
	virtual void onRead(int handle, const char* buffer, int len);

	//网络断开数据通知，reason：失败处理的缘由
	virtual void onClose(int handle, int reason);

	//线程回调函数
	void run(void *param);

	// 是否开启测试数据，加密四次握手成功后，发送一条测试数据给服务端；
	void set_test(bool on);

	bool get_test();

	void set_test_msg(const std::string &test_msg);

	std::string &get_test_msg();

private:

	//事件循环
	void event_loop();

	//定时处理流程
	void time_process(time_t now);

	//生成handle的方式，交由TcpConnection使用
	int make_handle();

private:

	SockEvent *_sock_event;
	OnlineUser _online_user;
	ThreadManager _threadmgr;
	bool _stop;
	bool _test;
	//加密传输的测试数据
	std::string _test_msg;
};

} /* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_NETWORK_H_ */
