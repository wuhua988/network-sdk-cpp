/*****************************************************************************
 Name        : interface.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_INTERFACE_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_INTERFACE_H_

#include <stdio.h>

//连接正常
#define NET_STATE_CONNECTED      (0x00)

//连接关闭（包含找不到handle，handle处于未连接状态）
#define NET_STATE_CLOSED         (0x01)

//正在连接状态
#define NET_STATE_CONNECTING     (0x02)

//查询的handle为非法handle
#define NET_STATE_INVALID_HANDLE (0x03)

//解析的每个PDU包的数据长度的最大值，超过这个最大值认为是异常包，关闭连接；
#define MAX_PDU_LEN (1024 * 1024)

enum
{
	SOCKET_ERROR_CONN_TIMEOUT = 1, //连接超时
	SOCKET_ERROR_TIMEOUT,          //长时间没有的活动的连接超时
	SOCKET_ERROR_CONN_FAIL,        //连接失败
	SOCKET_ERROR_PARSE_PDU,        //PDU解包错误
	SOCKET_ERROR_REMOTE_CLOSE,     //对端主动关闭
	SOCKET_ERROR_CLOSE             //监测是网络断开
};

class IService
{
public:
	virtual ~IService()
	{
	}

	//连接结果通知
	virtual void onConnect(int nHandle) = 0;

	//数据到来通知，交付出来的已经是一个完整的PDU包
	virtual void onRead(int nHandle, const char* pBuf, int nLen) = 0;

	//网络断开数据通知，reason：失败处理的缘由
	virtual void onClose(int nHandle, int nReason) = 0;
};

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_INTERFACE_H_ */
