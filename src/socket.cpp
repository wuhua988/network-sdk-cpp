/*****************************************************************************
 Name        : socket.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

#include <list>
#include <map>
#include <queue>
#include <vector>
#include <string>
//#include <ext/hash_map>
#include <sys/types.h>
#include <arpa/inet.h>

#include "mutex.h"
#include "socket.h"
#include "log.h"

namespace cppnetwork
{
Socket::Socket()
{
	_fd = -1;
}

Socket::~Socket()
{
	this->close();
}

bool Socket::set_address(const char *host, unsigned short port)
{
	struct sockaddr_in *dest = (&_address);

	memset(dest, 0, sizeof(struct sockaddr_in));

	dest->sin_family = AF_INET;
	dest->sin_port = htons(static_cast<short>(port));

	bool rc = true;
	if (host == NULL || host[0] == '\0') {
		dest->sin_addr.s_addr = htonl(INADDR_ANY);
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
			dest->sin_addr.s_addr = inet_addr(host);
		} else {
			static Mutex s_dns_mutex;
			s_dns_mutex.lock();
			struct hostent *host_ent = gethostbyname(host);
			if (host_ent != NULL) {
				memcpy(&(dest->sin_addr), *(host_ent->h_addr_list), sizeof(struct in_addr));
			} else {
				rc = false;
			}
			s_dns_mutex.unlock();
		}
	}

	return rc;
}

int Socket::connect()
{
	TRACE(__PRETTY_FUNCTION__);

	if (_fd > 0) {
		::close(_fd);
		_fd = -1;
	}

	int num = 3;
	//在android手机环境下对于环境建立的情况，多尝试几次； 2015-10-09， sotter
	while (_fd < 0 && num--) {
		_fd = socket(AF_INET, SOCK_STREAM, 0);
	}

	if (_fd < 0) {
		LOGW("fd < 0, sockete fail!!!");
		return 2;
	}

	set_so_blocking(false);
	socklen_t len = sizeof(_address);
	int ret = ::connect(_fd, (struct sockaddr*) &_address, len);
	if (ret == 0) {
		LOGI("Socket::connect success");
		return 0;
	} else if (errno == EINPROGRESS) {
		LOGI("Socket::connect EINPROGRESS continue");
		return 1;
	} else {
		LOGI("Socket::connect errno:%d: %s", errno, strerror(errno));;
		return 2;
	}
}

void Socket::close()
{
	TRACE(__PRETTY_FUNCTION__);

	if (_fd > 0) {
		::close(_fd);
		_fd = -1;
	}
}

bool Socket::set_int_option(int option, int value)
{
	return (setsockopt(_fd, SOL_SOCKET, option, (const void *) (&value), sizeof(value)) == 0);
}

bool Socket::set_time_option(int option, int milliseconds)
{
	struct timeval timeout;
	timeout.tv_sec = (int) (milliseconds / 1000);
	timeout.tv_usec = (milliseconds % 1000) * 1000000;
	return (setsockopt(_fd, SOL_SOCKET, option, (const void *) (&timeout), sizeof(timeout)) == 0);
}

bool Socket::set_keep_alive(bool on)
{
	return set_int_option(SO_KEEPALIVE, on ? 1 : 0);
}

bool Socket::set_reuse_addr(bool on)
{
	return set_int_option(SO_REUSEADDR, on ? 1 : 0);
}

bool Socket::set_solinger(bool on, int seconds)
{
	struct linger linger_time;
	linger_time.l_onoff = on ? 1 : 0;
	linger_time.l_linger = seconds;
	return (setsockopt(_fd, SOL_SOCKET, SO_LINGER, (const void *) (&linger_time), sizeof(linger_time)) == 0);
}

bool Socket::set_tcp_nodelay(bool nodelay)
{
	int noDelayInt = nodelay ? 1 : 0;
	return (setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (const void *) (&noDelayInt), sizeof(noDelayInt)) == 0);
}

bool Socket::set_so_blocking(bool on)
{
	bool rc = false;

	int flags = fcntl(_fd, F_GETFL, NULL);
	if (flags >= 0) {
		if (on) {
			flags &= ~O_NONBLOCK; // clear nonblocking
		} else {
			flags |= O_NONBLOCK;  // set nonblocking
		}

		if (fcntl(_fd, F_SETFL, flags) >= 0) {
			rc = true;
		}
	}

	return rc;
}

int Socket::get_soerror()
{
	if (_fd == -1)
		return EINVAL;

	int last_error = errno;

	int soerror = 0;
	socklen_t soerror_len = sizeof(soerror);
	if (getsockopt(_fd, SOL_SOCKET, SO_ERROR, (void *) (&soerror), &soerror_len) != 0) {
		return last_error;
	}

	if (soerror_len != sizeof(soerror))
		return EINVAL;

	return soerror;
}

int Socket::read(void *data, int len)
{
	if (_fd == -1)
		return -1;

	int res;
	do {
		res = (int) ::read(_fd, data, len);
	} while (res < 0 && errno == EINTR);

	return res;
}

int Socket::write(const void *data, int len)
{
	if (_fd == -1) {
		return -1;
	}
	int res;
	do {
		res = (int) ::write(_fd, data, len);
	} while (res < 0 && errno == EINTR);

	return res;
}

int Socket::getfd()
{
	return _fd;
}

} /* namespace cppnetwork */
