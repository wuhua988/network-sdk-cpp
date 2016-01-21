/*****************************************************************************
 Name        : netcheck.cpp
 Author      : sotter
 Date        : 2015年6月9日
 Description :
 ******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "tools.h"
#include "netcheck.h"
#include <sys/ioctl.h>
#include "log.h"

using namespace cppnetwork;
static string int2string(const int i)
{
	char buffer[16] = { 0 };
	snprintf(buffer, sizeof(buffer) - 1, "%d", i);
	return buffer;
}

//在没有的编译器优化的情况下效率较低，不适合在高性能要求下使用
static void json_append(string &json, const char *key, int value)
{
	if (!json.empty())
		json += ",";
	json = json + "\"" + key + "\"" + ":" + int2string(value);
}

static void json_append(string &json, const char *key, const char *value)
{
	if (!json.empty())
		json += ",";
	json = json + "\"" + key + "\"" + ":" + "\"" + value + "\"";
}

//根据开始时间和结束时间，计算消耗的时间，结果为ms
static int use_time(timeval &begin, timeval &end)
{
	return (int) (((end.tv_sec - begin.tv_sec) * 1000) + ((end.tv_usec - begin.tv_usec) / 1000));
}

static int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option; //注意返回就的文件描述符属性以便将来恢复文件描述符属性
}

NetCheck::NetCheck()
{
	_sock_fd = -1;
	_connected = false;
	_conn_timeout = 3 * 1000;
	_send_timeout = 3 * 1000;
	_recv_timeout = 3 * 1000;
	memset(&_set_address, 0, sizeof(_set_address));
}

NetCheck::~NetCheck()
{
	disconnection();
}

bool NetCheck::set_address(const char *host, unsigned short port)
{
	memset(static_cast<void *>(&_address), 0, sizeof(_address));

	_result.server_port = port;
	_address.sin_family = AF_INET;
	_address.sin_port = htons(static_cast<short>(port));

	bool rc = true;
	if (host == NULL || host[0] == '\0') {
		_address.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		char c;
		const char *p = host;
		bool is_ipaddr = true;
		while ((c = (*p++)) != '\0') {
			if ((c != '.') && (!((c >= '0') && (c <= '9')))) {
				is_ipaddr = false;
				break;
			}
		}

		if (is_ipaddr) {
			_result.server_host = host;
			_address.sin_addr.s_addr = inet_addr(host);
		} else {
			struct timeval begin, end;
			gettimeofday(&begin, NULL);
			struct hostent *hostname = gethostbyname(host);
			gettimeofday(&end, NULL);

			_result.parse_dns_use_time = use_time(begin, end);

			if (hostname != NULL) {
				_result.parse_dns_result = OK;
				memcpy(&(_address.sin_addr), *(hostname->h_addr_list), sizeof(struct in_addr));
				_result.server_host = inet_ntoa(_address.sin_addr);
				LOGI("GET DNS OK, %s => %s", host, _result.server_host.c_str());
			} else {
				_result.parse_dns_result = PARSE_DNS_FAIL;
				rc = false;
			}
		}
	}

	_set_address = rc;
	return rc;
}

std::string NetCheck::convert_result()
{
	string json_result;
	json_append(json_result, "parse_dns_result", _result.parse_dns_result);
	json_append(json_result, "server_host", _result.server_host.c_str());
	json_append(json_result, "server_port", _result.server_port);
	json_append(json_result, "parse_dns_use_time", _result.parse_dns_use_time);
	json_append(json_result, "connect_result", _result.connect_result);
	json_append(json_result, "connect_time", _result.connect_time);
	json_append(json_result, "send_req_result", _result.send_req_result);
	json_append(json_result, "send_req_bytes", _result.send_req_bytes);
	json_append(json_result, "recv_rsp_result", _result.recv_rsp_result);
	json_append(json_result, "recv_rsp_bytes", _result.recv_rsp_bytes);
	json_append(json_result, "send_recv_time", _result.send_recv_time);
	json_append(json_result, "error", _result.error);
	json_append(json_result, "errstr", strerror(_result.error));
//	json_append(json_result, "recv_data", _result.recv_data.c_str());

	json_result = "{" + json_result + "}";

	return json_result;
}

//执行检测，并将结果在result中返回
std::string NetCheck::excute(const char *host, unsigned short port, const char *buffer, int len)
{
	char recv_buffer[1024 * 10];

	if (connect(host, port, 3 * 1000) < 0)
		goto Done;

	if (send_tmout(_sock_fd, buffer, len, 3 * 1000) < 0)
		goto Done;

	if (recv_tmout(_sock_fd, recv_buffer, sizeof(recv_buffer), 3 * 1000) < 0)
		goto Done;

	Done:
	//检测结束时，必须将socket关闭掉
	disconnection();
	return convert_result();
}

//JUST FOR TEST
void NetCheck::show_result()
{
	printf("##### CHECK RESULT ############\n");
	printf(" parse_dns : %d\n", _result.parse_dns_result);
	printf(" parse_dns host : %s\n", _result.server_host.c_str());
	printf(" parse_dns port : %d\n", (int) (_result.server_port));
	printf(" parse_dns use %d ms\n", _result.parse_dns_use_time);

	printf(" connect result : %d\n", _result.connect_result);
	printf(" connect use %d ms\n", _result.connect_time);
	printf(" send : %d\n", _result.send_req_result);

	printf(" recv result: %d\n", _result.recv_rsp_result);
	printf(" recvbytes : %d\n", _result.recv_rsp_bytes);
	printf(" send-recv time: %d ms\n", _result.send_recv_time);

	printf(" errno : %d\n", _result.error);
	printf(" server_host : %s\n", _result.server_host.c_str());

	fflush(stdout);
	return;
}

int NetCheck::connect(const char *host, unsigned short port, int timeout)
{
	if (!set_address(host, port)) {
		//想当于没有跑到connect，直接返回
		return -1;
	}

	_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock_fd < 0) {
		_result.connect_result = SOCK_ERROR;
		return -1;
	}

	setnonblocking(_sock_fd);
	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	int ret = ::connect(_sock_fd, (struct sockaddr*) &_address, sizeof(_address)); //connect连接服务端
	if (ret == 0) {
		//若connect成功返回则表明连接立即建立，这种情况可能出现在本机连接本机
		gettimeofday(&end, NULL);
		LOGD("connect success");
		_result.connect_result = OK;
		return 0;
	} else if (errno != EINPROGRESS) {
		gettimeofday(&end, NULL);
		_result.error = errno;
		_result.connect_result = SOCK_CONN_FAIL;
		LOGI("connect fail errno %d : %s", errno, strerror(errno));
		return -1;
	}

	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(_sock_fd, &writefds);

	struct timeval tmout;
	tmout.tv_sec = timeout / 1000;
	tmout.tv_usec = (timeout % 1000) * 1000;
	ret = select(_sock_fd + 1, NULL, &writefds, NULL, &tmout); //select监听sockfd上在超时时间timeout内是否可写
	gettimeofday(&end, NULL);
	if (ret <= 0) {
		_result.connect_result = SOCK_CONN_TIMEOUT;
		LOGD("select ret: %d", ret);
		//若可写事件没有发生则连接超时，返回-1
		return -1;
	}

	//开始检测connect返回结果
	int error = 0;
	socklen_t length = sizeof(error);
	if (getsockopt(_sock_fd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) {
		LOGD("getsockopt fail %d:%s", errno, strerror(errno));
		_result.connect_result = SOCK_CONN_FAIL;
		return -1;
	} else if (error > 0) {
		_result.connect_result = SOCK_CONN_FAIL;
		_result.error = error;
		LOGD("connect after select fail, %d:%s", errno, strerror(errno));
		return -1;
	} else if (error == 0) {
		_result.connect_result = OK;
		LOGD("connect error = 0 , success");
		_connected = true;

		return 0;
	} else {
		printf("######  不能跑到这！！！！########");
	}

	return 0;
}

void NetCheck::disconnection()
{
	if (_sock_fd > 0) {
		::close(_sock_fd);
		_sock_fd = -1;
		_connected = false;
	}

	return;
}

int NetCheck::recv_tmout(int sockfd, char *buffer, int buffer_len, int timeout)
{
	if (sockfd < 0) {
		_result.recv_rsp_result = SOCK_RECV_CONN_ERROR;
		return -1;
	}
	if (buffer == NULL || buffer_len <= 0) {
		_result.recv_rsp_result = SOCK_RECV_PARAM_ERROR;
		return -1;
	}
	struct timeval tval;
	tval.tv_sec = timeout / 1000;
	tval.tv_usec = (timeout % 1000) * 1000;

	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(sockfd, &read_fds);
	int res = select(sockfd + 1, &read_fds, NULL, NULL, &tval);
	int offset = 0;
	memset(buffer, 0, buffer_len);
	if (res == 1) {
		while (offset < buffer_len - 1) {
			int recv_bytes = (int) ::recv(sockfd, (void*) buffer, buffer_len - offset, 0);

			if (recv_bytes > 0) {
				_result.recv_rsp_result = OK;
				_result.recv_rsp_bytes += recv_bytes;
				offset += recv_bytes;
				continue;
			} else if (recv_bytes == 0) {
				printf("#############recv_bytes #################\n");
				_result.recv_rsp_result = SOCK_RECV_CONN_ERROR;
				break;
			} else {
				if (errno != EAGAIN) {
					_result.recv_rsp_result = SOCK_RECV_SYS_ERROR;
					_result.error = errno;
				} else {
					_result.recv_rsp_result = OK;
				}

				break;
			}
		}
	} else if (res == 0) {
		_result.recv_rsp_result = SOCK_RECV_TIMEOUT;
		LOGD("SOCK RECV timetout after %dms", timeout);
	} else if (res < 0) {
		_result.recv_rsp_result = SOCK_RECV_SYS_ERROR;
	}

	if (_result.recv_rsp_result == OK) {
		gettimeofday(&_result.recv_success_time, NULL);
		_result.recv_data = buffer;
		_result.send_recv_time = use_time(_result.send_success_time, _result.recv_success_time);
	}

	return _result.recv_rsp_result == OK ? 0 : -1;
}

int NetCheck::send_tmout(int sockfd, const char *buffer, int buffer_len, int timeout)
{
	if (sockfd < 0) {
		_result.send_req_result = SOCK_SEND_CONN_ERROR;
		return -1;
	}

	if (buffer == NULL || buffer_len < 0) {
		_result.send_req_result = SOCK_SEND_PARAM_ERROR;
		return -1;
	}

	int len = 0;
	fd_set write_fds;
	int offset = 0;
	struct timeval tval;

	tval.tv_sec = timeout / 1000;
	tval.tv_usec = (timeout % 1000) * 1000;

	while (offset < buffer_len) {
		len = (int) ::write(sockfd, buffer + offset, buffer_len - offset);
		if (len > 0) {
			offset += len;
			continue;
		} else {
			if (errno == EAGAIN) {
				FD_ZERO(&write_fds);
				FD_SET(sockfd, &write_fds);
				int res = select(sockfd + 1, NULL, &write_fds, NULL, &tval);
				if (res > 0) {
					continue;
				} else if (res < 0) {
					_result.send_req_result = SOCK_SEND_SYS_ERROR;
					_result.error = errno;
					LOGI("select error-%d-%s", errno, strerror(errno));
				} else {
					LOGI("%s", "select timeout");
					break;
				}
			} else {
				_result.send_req_result = SOCK_SEND_SYS_ERROR;
				break;
			}
		}
	}

	if (offset == buffer_len) {
		LOGD("send %d bytes success", offset);
		gettimeofday(&_result.send_success_time, NULL);
		_result.send_req_result = OK;
	}

	return _result.send_req_result == OK ? 0 : -1;
}
