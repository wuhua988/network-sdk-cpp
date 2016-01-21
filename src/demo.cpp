/*****************************************************************************
 Name        : demo.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#include <stdio.h>
#include <map>
#include <sys/resource.h>
#include "mutex.h"
#include "socket.h"
#include "thread.h"
#include "network.h"
#include "ping.h"

using namespace std;
using namespace cppnetwork;

class User
{
public:
	User()
	{
		_online = false;
	}
	void set_online()
	{
		_online = true;
	}
	void set_offline()
	{
		_online = false;
	}
	bool get_online()
	{
		return _online;
	}

	bool _online;
};

class TestOnlineUser
{
public:

	void set_online(int handle)
	{
		_mutex.lock();
		_users[handle].set_online();
		_mutex.unlock();
	}

	void set_offline(int handle)
	{
		_mutex.lock();
		_users[handle].set_offline();
		_mutex.unlock();
	}

	void display()
	{
		int offline_users = 0;
		int online_users = 0;

		_mutex.lock();
		std::map<int, User>::iterator iter = _users.begin();
		for (; iter != _users.end(); ++iter) {
			if (iter->second.get_online()) {
				online_users++;
			} else {
				offline_users++;
			}
		}
		_mutex.unlock();

//		printf("time:%d online:%d, offline:%d \n",
//				time(NULL), online_users, offline_users);
		fflush(stdout);
	}

private:
	std::map<int, User> _users;
	Mutex _mutex;
};

class TestNetWork: public NetWork
{
public:

	TestNetWork()
	{
		_timeout_num = 0;
	}

	//连接结果通知
	virtual void onConnect(int handle)
	{
		_online_users.set_online(handle);
	}

	//数据到来通知，交付出来的已经是一个完整的PDU包
	virtual void onRead(int handle, const char* buffer, int len)
	{

	}

	//网络断开数据通知，reason：失败处理的缘由
	virtual void onClose(int handle, int reason)
	{
		if (reason == 1) {
			_timeout_num++;
		}
		_online_users.set_offline(handle);
	}

	virtual void display()
	{
		_online_users.display();
		printf("timeout:%d\n", _timeout_num);
	}

	TestOnlineUser _online_users;

	int _timeout_num;
};

#define IP  "10.0.30.125"
#define PORT 7001
#define TESTNUM 1000

int main(int argc, char *argv[])
{
	TestNetWork net;

	net.init();

//	net.set_test(true);
//	net.set_test_msg("=====GOOD LUCK=====");

//	int num = TESTNUM;
//
//	if(argc >= 2)
//	{
//		num = atoi(argv[1]);
//	}
//
//	while(num--)
//	{
//		net.connect(IP, PORT, false);
//	}
//
//	while(true)
//	{
//		net.display();
//		sleep(1);
//	}
	int handle = net.connect("127.0.0.1", 7001);
	printf("handle = %d \n", handle);
	sleep(1);
	net.close(handle);

	handle = net.connect("127.0.0.1", 7001);
	printf("handle = %d \n", handle);
	sleep(1);

	net.close(handle);

	handle = net.connect("127.0.0.1", 7001);
	printf("handle = %d \n", handle);

	sleep(1);

	net.close(handle);

	handle = net.connect("127.0.0.1", 7001);
	printf("handle = %d \n", handle);

	sleep(1);

	net.close(handle);

	handle = net.connect("127.0.0.1", 7001);
	printf("handle = %d \n", handle);

	sleep(10000);

	//再次传输加密数据测试
//	cppnetwork::CryptTestMsg test_msg;
//	test_msg.set_msg("1234567890");
//	DataBuffer buffer ;
//	test_msg.Body(buffer);
//
//	fflush(stdout);
//	sleep(3);
//
//	net.send(handle, buffer.getData(), buffer.getDataLen());
//
//	sleep(3000);
//	fflush(stdout);

//	//正常连接IP测试
//	int handle = net.connect("127.0.0.1", 4312);
//
//	//测试一个不通的IP
//	int handle1 = net.connect("198.168.0.1", 4312);
//
//	//测试端口不通的情况
//	int handle2 = net.connect("127.0.0.1", 4001);
//
//	//测试相同连接，建立两个；
//	int handle3 = net.connect("127.0.0.1", 4312);
//
//	//测试域名的情况
//	int handle4 = net.connect("www.baidu.com", 80);
//
//	printf("connect %d %d %d %d %d \n", handle, handle1, handle2, handle3, handle4);
//
//	bool ret = net.send(handle, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle, ret ? "success":"fail");
//
//	ret = net.send(handle1, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle1, ret ? "success":"fail");
//
//	ret = net.send(handle2, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle2, ret ? "success":"fail");
//
//	ret = net.send(handle3, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle3, ret ? "success":"fail");
//
//	ret = net.send(handle4, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle4, ret ? "success":"fail");
//
//	//等待handle2连接超时
//	sleep(5);
//
//	net.close(handle);
//
//	net.close(handle1);
//
//	net.close(handle2);
//
//	net.close(handle3);
//
//	net.close(handle4);
//
//	/*close 完毕后继续调用发送数据，监测错误 */
//	ret = net.send(handle, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle, ret ? "success":"fail");
//
//	ret = net.send(handle1, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle1, ret ? "success":"fail");
//
//	ret = net.send(handle2, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle2, ret ? "success":"fail");
//
//	ret = net.send(handle3, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle3, ret ? "success":"fail");
//
//	ret = net.send(handle4, "GOOD LUCK", 9);
//	printf("%s:%d send handle:%d %s \n", __FILE__, __LINE__, handle4, ret ? "success":"fail");
//
//sleep(1);
//	const char *host[] = {"127.0.0.1", "www.baidu.com", "192.168.0.1"};

//	cppnetwork::Ping ping = cppnetwork::Ping();
//
//	for (size_t i = 0; i < (sizeof(host)) / sizeof(char*); i++)
//	{
//		printf("result:%s \n", ping.ping(host[i]).c_str());
//	}

	return 0;
}

