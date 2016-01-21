/*****************************************************************************
 Name        : tools.h
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#ifndef APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TOOLS_H_
#define APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TOOLS_H_

#include <stdio.h>
#include <stdint.h>

typedef unsigned char uchar_t;

extern void WriteInt16(uchar_t *buf, int16_t data);
extern void WriteUint16(uchar_t *buf, uint16_t data);
extern void WriteInt32(uchar_t *buf, int32_t data);
extern void WriteUint32(uchar_t *buf, uint32_t data);
extern void WriteInt64(uchar_t *buf, int64_t data);
extern void WriteUint64(uchar_t *buf, uint64_t data);
extern int16_t ReadInt16(uchar_t *buf);
extern uint16_t ReadUint16(uchar_t* buf);
extern int32_t ReadInt32(uchar_t *buf);
extern uint32_t ReadUint32(uchar_t *buf);
extern int64_t ReadInt64(uchar_t *buf);
extern uint64_t ReadUint64(uchar_t *buf);

extern void show_hex(const char *data, int len, const char *str);

class Ref
{
public:
	Ref() :
			_ref(1)
	{
	}
	virtual ~Ref()
	{
	}

	// 添加引用
	int add_ref();

	// 取得引用
	int get_ref();

	// 释放引用
	void release();

	bool is_shared();

private:
	// 引用记数值
	unsigned int _ref;
};

#endif /* APP_SRC_IOSNETWORK_IOSNETWORK_0_2_TOOLS_H_ */
