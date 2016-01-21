/*****************************************************************************
 Name        : tcpconnection.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include "network.h"
//#include "pdu_crypt.h"
//#include "im_pdu_crypt.h"
#include "tcpconnection.h"
#include "log.h"

#define MIN_PDU_LEN   (4)

namespace cppnetwork
{
static uint32_t ReadUint32(unsigned char *buf)
{
	uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

bool IsPduAvailable(unsigned char* buf, uint32_t len, uint32_t& pdu_len)
{
	if (len < 4)
		return false;

	pdu_len = ReadUint32(buf);
	if (pdu_len > len || pdu_len < MIN_PDU_LEN)
		return false;

	return true;
}

TcpConnection::TcpConnection(NetWork *n)
{
	TRACE(__PRETTY_FUNCTION__);

	_port = 0;
	_network = n;
	_handle = n->make_handle();
	_last_active_time = time(0);

	_last_error = 0;
	_encrypt = false;
	//以时间作为种子的随机数
	srand(time(NULL));
}

TcpConnection::~TcpConnection()
{
	TRACE(__PRETTY_FUNCTION__);
	//_sock销毁的时候自己调用_sock->close, 这里不需要主动调用了
}

bool TcpConnection::connect(const char *host, unsigned short port)
{
	_ip = host;
	_port = port;

	TRACE(__PRETTY_FUNCTION__);

	if (!_sock.set_address(host, port)) {
		LOGW("set address host:%s port:%d fail!", host, port);
		return false;
	}

	int ret = _sock.connect();

	if (ret == 0 || ret == 1) {
		_last_active_time = time(0);
		_state = CONNECTING;
		return true;
	} else {
		_state = CLOSED;
		return false;
	}
}

void TcpConnection::on_read_event()
{
	TRACE(__PRETTY_FUNCTION__);

	if (_state != CONNECTED && _state != CRYPTREGING)
		return;

	if (!read_data()) {
		_network->close(_handle);
		_network->onClose(_handle, get_last_error());
	}

	return;
}

bool TcpConnection::read_data()
{
	TRACE(__PRETTY_FUNCTION__);

	bool broken = false;
	//数据读取的时候，上层已经引入计数+1，所以不用担心被删掉
	//每个包设置为8K的大小，第一次读取出来最大是8K
	_input.ensureFree(READ_WRITE_SIZE);
	//只有读到数据才认为连接是正常的
	_last_active_time = time(0);
	//ret表示已经读取到的数据
	int ret = _sock.read(_input.getFree(), _input.getFreeLen());

	int read_cnt = 0;
	uint32_t pdu_len = 0;
	while (ret > 0) {
		_input.pourData(ret);
		while (IsPduAvailable((unsigned char*) _input.getData(), _input.getDataLen(), pdu_len)) {
			//如果有包可读，立马交由上层处理
			on_read(_input.getData(), pdu_len);
			_input.drainData(pdu_len);
		}

		//如果发现一个超过8M的包，直接断开,如果一个包超过了8M还没完，只能说明这个协议设计的太垃圾，需要协议层面分包！
		if (pdu_len > 8 * 1024 * 1024 || pdu_len <= MIN_PDU_LEN) {
			LOGW("parse pdu_len %d > 1024 * 1024 pdu_len <= MIN_PDU_LEN, broken", pdu_len);
			set_last_error(SOCKET_ERROR_PARSE_PDU);
			broken = true;
			break;
		}

		//如果发生了断开事件，或是_input没有读满（说明缓冲区里面已经没有数据了）
		if (broken || _input.getFreeLen() > 0 || read_cnt >= 10)
			break;

		//如果判定读出来的包还没有解析完全说明，可能有未读出来的半包。
		//原先的判断条件为decode > 0, 修改为decode >= 0, decode == 0时可能是一个大包
		//数据包还没有完全接收完毕
		if (pdu_len > (uint32_t) _input.getDataLen()) {
			_input.ensureFree(READ_WRITE_SIZE);
		}

		//todo: 如果发送是一个大包，要在encode体现出来，告知上层，那么_input继续扩大自己的范围来适应大包的发送。
		ret = _sock.read(_input.getFree(), _input.getFreeLen());

		read_cnt++;
	}

	//将读缓存区回归到初始位置
	_input.shrink();

	/*************
	 * broken事件在最后处理，且是在事件在外层调用的，所以当读取几个包后，读到断开事件时并不影响已经读到的数据的处理。但需要一个前提条件：
	 * （1）断开事件不能采用直接回调的方式，而是跟其它packet数据一样进行处理排队，这个地方要对packet进行处理
	 *************/
	if (!broken) {
		if (ret == 0) {
			set_last_error(SOCKET_ERROR_REMOTE_CLOSE);
			broken = true;
		} else if (ret < 0) {
			int error = _sock.get_soerror();
			if (error != EAGAIN) {
				broken = true;
				set_last_error(SOCKET_ERROR_CLOSE);
			}
			broken = (error != EAGAIN);
		}
	}

	return !broken;
}

void TcpConnection::on_write_event()
{
	TRACE(__PRETTY_FUNCTION__);
	if (getstate() == CONNECTING) {
		//处于正在连接状态
		int error = _sock.get_soerror();
		if (error == 0) //连接成功
				{
			//todo:设置可写读事件进去，废除可写事件
			on_connect();
		} else {
			set_last_error(SOCKET_ERROR_CONN_FAIL);
			LOGW("on_write_event: connect %s:%d fail, fd:%d errno:%d", _ip.c_str(), _port, _sock.getfd(), error);
			//(0)设置状态CLOSED
			//(1)从sockevent中删除
			//(2)network的onlineuser中清除
			//(3)this->close();
			//(4)回调app on_close事件
			_network->close(_handle);
			_network->onClose(_handle, get_last_error());
		}
	} else if (getstate() == CONNECTED || getstate() == CRYPTREGING) {
		write_data();
	} else {
		//ERROR: handle:1 fd:-1 state:1, 说明已经close的函数又被触发到了
		LOGE("can't arrive here!!!!! handle:%d fd:%d state:%d %s:%d", gethandle(), getfd(), getstate(), __FILE__,
				__LINE__);
	}

	return;
	//数据写入
}

void TcpConnection::write_data()
{
	TRACE(__PRETTY_FUNCTION__);

	//写数据的时候锁起来，防止与deliver_data发生冲突
	Guard g(_mutex);

	//如果write出现ERRORAGAIN的情况，下一个包继续发送；
	int writeCnt = 0;
	int ret = 0;

	do {
		if (_output.getDataLen() == 0) {
			break;
		}

		// write data
		int ret = _sock.write((const void*) _output.getData(), _output.getDataLen());
		if (ret > 0) {
			//投递缓冲区成功也认为是连接正常的
			_last_active_time = time(0);
			_output.drainData(ret);
		}
		writeCnt++;
		/*******
		 * _output.getDataLen() == 0 说明发送的数据都结束了
		 * 停止发送的条件：
		 * (1)发送的结果ret <= 0, 发送失败，或者写缓冲区已经满了。
		 * (2)_output.getDataLen() > 0 说明一次没有发送完，还有没有发送完的数据。就直接退出来停止发送了。
		 * 	 那么这块数据去了哪里？
		 **********/
		//todo:
	} while (ret > 0 && _output.getDataLen() == 0 && writeCnt < 10);

	_output.shrink();

	//如果全部发送完成，那么将写事件清除掉
	if (_output.getDataLen() <= 0) {
		_network->_sock_event->set_event(this, true, false);
	}

	return;
}

bool TcpConnection::deliver_data(const char *data, int len)
{
	TRACE(__PRETTY_FUNCTION__);
	Guard g(_mutex);

	//如果是出于非连接状态，发送数据是没有用的
	if (_state != CONNECTED)
		return false;

	_output.writeBytes(data, len);
	_network->_sock_event->set_event(this, true, true);
	return true;
}

bool TcpConnection::deliver_data_inter(const char *data, int len, bool encrypt)
{
	Guard g(_mutex);

	//对于内部调用而言在CONNECTING, CRYPTREGING状态下也是可以发送数据的
	if (_state == CLOSED || _state == WAIT_CLOSE)
		return false;

	_output.writeBytes(data, len);

	_network->_sock_event->set_event(this, true, true);
	return true;
}

void TcpConnection::close()
{
	TRACE(__PRETTY_FUNCTION__);

	_state = CLOSED;
	_sock.close();

	return;
}

//当读取到数据时的处理
void TcpConnection::on_read(const char* buffer, int len)
{
	TRACE(__PRETTY_FUNCTION__);

	_network->onRead(_handle, buffer, len);

	return;
}

void TcpConnection::on_connect()
{
	TRACE(__PRETTY_FUNCTION__);

//	if(get_encrypt())
//	{
//		_state = CRYPTREGING;
//		to_crypt_reg_req();
//	}
//	else
//	{
	_state = CONNECTED;
	_network->onConnect(_handle);
//	}
}

////发送加密注册请求，获取RSA公钥
//void TcpConnection::to_crypt_reg_req()
//{
//	TRACE(__PRETTY_FUNCTION__);
//
//	//todo: 初始化reg_req的值
//	CryptRegReq reg_req;
//
//	DataBuffer buffer;
//	reg_req.Body(buffer);
//	deliver_data_inter(buffer.getData(), buffer.getDataLen(), false);
//	return;
//}

////发送TEA加密密钥
//void TcpConnection::to_crypt_key_req()
//{
//	TRACE(__PRETTY_FUNCTION__);
//
//	//todo: 初始化key_req的值
//	CryptKeyReq key_req;
//
//	key_req.set_tea_key(_tea_key);
//
//	DataBuffer buffer, crypt_buffer;
//	key_req.Body(buffer);
//	PduCrypt::PduRSAEncrypt(_public_key, buffer.getData(), buffer.getDataLen(), crypt_buffer);
//
//	//采用RSA加密投递数据
//	deliver_data_inter(crypt_buffer.getData(), crypt_buffer.getDataLen(), false);
//	return;
//}
//
////加密注册响应消息
//void TcpConnection::on_crypt_reg_rsp(DataBuffer &rsp)
//{
//	TRACE(__PRETTY_FUNCTION__);
//
//	CryptRegRsp reg_rsp;
//	reg_rsp.UnBody(rsp);
//	_public_key =reg_rsp.get_rsa_public_key();
//
//	LOGI("receive public key:%s", _public_key.c_str());
//	to_crypt_key_req();
//}
//
////server收到后的响应消息
//void TcpConnection::on_crypt_rsakey_rsp(DataBuffer &rsp)
//{
//	TRACE(__PRETTY_FUNCTION__);
//
//	CryptKeyRsp rsa_rsp;
//	rsa_rsp.UnBody(rsp);
//
//	//这是时候才回调连接建立成功
//	_state = CONNECTED;
//	_network->onConnect(_handle);
//
//	//JUST FOR TEST 发送一个测试消息
//	if(_network->get_test())
//	{
//		CryptTestMsg msg;
//		msg.set_msg(_network->get_test_msg());
//		DataBuffer out;
//		msg.Body(out);
//		deliver_data_inter(out.getData(), out.getDataLen(), true);
//	}
//
//	return;
//}
//
//void TcpConnection::on_crypt_test_msg(DataBuffer &rsp)
//{
//	TRACE(__PRETTY_FUNCTION__);
//
//	CryptTestMsg test_msg;
//	test_msg.UnBody(rsp);
//
//	printf("RECEIVE TEST MSG LEN %d:%s \n",
//			(int)test_msg.get_msg().length(), test_msg.get_msg().c_str());
//
//	fflush(stdout);
//}

} /* namespace cppnetwork */
