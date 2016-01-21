/*****************************************************************************
 Name        : tcpconnection.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TCPCONNECTION_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TCPCONNECTION_H_

#include "interface.h"
#include "socket.h"
#include "tools.h"
#include "databuffer.h"
//#include "mogucrypt.h"
#include "mutex.h"

namespace cppnetwork
{
enum CONNSTATE
{
	CLOSED = 1, CONNECTING,  // 正在异步连接中
	CRYPTREGING,    // 连接正在建立，加密认证中
	CONNECTED,   // 连接成功，对于加密而言，加密4次握手后才转移到此状态
	WAIT_CLOSE   // 代码中没有赋值
};

class NetWork;

//设置数据读取的buffer大小默认为8K
#define READ_WRITE_SIZE (8 * 1024)

//Connect的socket的关系，组合，而非继承也非聚合
class TcpConnection: public Ref
{
public:
	//tcpconnect建立的时候
	TcpConnection(NetWork *n);

	virtual ~TcpConnection();

//	void set_encrypt();

//	bool get_encrypt();

	bool connect(const char *host, unsigned short port);

	void on_read_event();

	void on_write_event();

	//flag表示是TCPConnection是否加密, 只有tcpconnection和network同时认为需要加密才加密
	bool deliver_data(const char *data, int len);

	void close();

	int getfd()
	{
		return _sock.getfd();
	}

	unsigned gethandle()
	{
		return _handle;
	}

	int getstate()
	{
		return _state;
	}

	time_t get_last_active_time()
	{
		return _last_active_time;
	}

	void set_last_error(int error)
	{
		_last_error = error;
	}

	int get_last_error()
	{
		return _last_error;
	}

private:

	bool read_data();

	void write_data();

	// 当读取到数据时的处理
	void on_read(const char* buffer, int len);

	// 连接建立时的本地调用
	void on_connect();

	//专供内部调用使用的数据发送
	bool deliver_data_inter(const char *data, int len, bool encrypt = true);

//	// 发送加密注册请求，获取RSA公钥
//	void to_crypt_reg_req();
//
//	// 发送TEA加密密钥
//	void to_crypt_key_req();
//
//	// 加密注册响应消息
//	void on_crypt_reg_rsp(DataBuffer &rsp);
//
//	// server收到后的响应消息
//	void on_crypt_rsakey_rsp(DataBuffer &rsp);
//
//	//  收到测试数据
//	void on_crypt_test_msg(DataBuffer &rsp);

private:

	string _ip;
	unsigned short _port;
	unsigned int _handle;
	CONNSTATE _state;
	Socket _sock;
	DataBuffer _input;
	DataBuffer _output;
	NetWork *_network;
	Mutex _mutex;
	time_t _last_active_time;

	int _last_error;
	bool _encrypt;
};

} /* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TCPCONNECTION_H_ */
