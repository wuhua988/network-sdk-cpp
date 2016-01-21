/*****************************************************************************
 Name        : netcheck.h
 Author      : sotter
 Date        : 2015年6月9日
 Description : 针对同一个netcheck对象，不允许多线程同时调用
 ******************************************************************************/

#ifndef NETCHECK_H_
#define NETCHECK_H_

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/time.h>
#include <string>

using namespace std;

#define OK                       (0)

//没有运行到那一步
#define NORUN                    (-1)

#define SOCK_REMOTE_CLOSE        (1)
#define SOCK_ERROR               (2)

#define PARSE_DNS_FAIL           (4)
//连接超时
#define SOCK_CONN_TIMEOUT        (5)
//此时具体原因放入到errno
#define SOCK_CONN_FAIL           (6)
//send送入的参数错误
#define SOCK_SEND_PARAM_ERROR    (7)
//连接不存在或已经断开
#define SOCK_SEND_CONN_ERROR     (8)
//select发生错误，错误码放入到errno中
#define SOCK_SEND_SYS_ERROR      (9)
//发送超时
#define SOCK_SEND_TIMEOUT        (10)
//读取时传递的参数错误
#define SOCK_RECV_PARAM_ERROR    (11)
//连接不存在或已经断开
#define SOCK_RECV_CONN_ERROR     (12)
//读取数据超时，注意：这里的超时指的是没有收到任何数据，不是收到数据不全
#define SOCK_RECV_TIMEOUT        (13)
//读取数据时发生系统错误，错误码放在Record中的errno中
#define SOCK_RECV_SYS_ERROR      (14)

#ifndef __NET_CHECK_RESULT_TYPE__
#define __NET_CHECK_RESULT_TYPE__
struct NetCheckResult
{
	int parse_dns_result;
	string server_host;
	unsigned short server_port;
	int parse_dns_use_time;

	int connect_result;
	int connect_time;

	int send_req_result;
	int send_req_bytes;
	struct timeval send_success_time;

	int recv_rsp_result;
	int recv_rsp_bytes;

	struct timeval recv_success_time;
	//从发送数据成功到接收数据成功这段时间
	int send_recv_time;

	string recv_data;

	int error;

	NetCheckResult()
	{
		server_port = 0;
		parse_dns_result = -1;
		parse_dns_use_time = 0;
		connect_result = -1;
		connect_time = 0;
		send_req_result = -1;
		send_req_bytes = 0;
		send_recv_time = 0;
		recv_rsp_result = -1;
		recv_rsp_bytes = 0;
		recv_rsp_bytes = 0;
		error = 0;
	}
};
#endif

class NetCheck
{
public:

	NetCheck();

	virtual ~NetCheck();

	//执行检测，并将结果在result中返回,result 采用json的方式
	string excute(const char *host, unsigned short port, const char *buffer, int len);

	//展示执行的结果, JUST FOR TEST
	void show_result();

private:

	//设置连接地址，host可以是IP也可以是域名，如果是域名会记录域名解析事件；
	bool set_address(const char *host, unsigned short port);

	std::string convert_result();

	int connect(const char *host, unsigned short port, int timeout);

	void disconnection();

	int send_tmout(int sockfd, const char *buffer, int buffer_len, int timeout);

	int recv_tmout(int sockfd, char *buffer, int buffer_len, int timeout);

private:

	int _sock_fd;

	bool _set_address;

	struct sockaddr_in _address;

	bool _connected;

	//超时时间设定，默认都是3s
	int _conn_timeout;
	int _send_timeout;
	int _recv_timeout;

	NetCheckResult _result;
};

#endif /* NETCHECK_H_ */
