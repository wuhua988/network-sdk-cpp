/*****************************************************************************
 Name        : socket.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKET_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKET_H_

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace cppnetwork
{

class Socket
{
public:
	Socket();

	virtual ~Socket();

	bool set_address(const char *host, unsigned short port);

	//nonblock connect, 0:succes, 1:would block, 2:fail
	int connect();

	void close();

	int read(void *data, int len);

	int write(const void *data, int len);

	bool set_keep_alive(bool on);

	bool set_reuse_addr(bool on);

	bool set_solinger(bool on, int seconds);

	bool set_tcp_nodelay(bool nodelay);

	bool set_time_option(int option, int milliseconds);

	bool set_so_blocking(bool on);

	int get_soerror();

	int getfd();

private:
	bool set_int_option(int option, int value);
private:

	int _fd;

	struct sockaddr_in _address;
};

} /* namespace cppnetwork */

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_SOCKET_H_ */
