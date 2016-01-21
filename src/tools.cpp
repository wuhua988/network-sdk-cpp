/*****************************************************************************
 Name        : tools.cpp
 Author      : sotter
 Date        : 2015年6月11日
 Description : 
 ******************************************************************************/

#include "mutex.h"
#include "tools.h"

static cppnetwork::Mutex g_mutex;

/**
 * 添加引用
 */
int Ref::add_ref()
{
	g_mutex.lock();
	++_ref;
	g_mutex.unlock();

	return _ref;
}

/**
 * 取得引用
 */
int Ref::get_ref()
{
	cppnetwork::Guard g(g_mutex);
	return _ref;
}

/**
 * 释放引用
 */
void Ref::release()
{
	bool destory = false;
	{
		g_mutex.lock();

		--_ref;
		if (_ref == 0) {
			destory = true;
		}
		g_mutex.unlock();
	}
	if (destory) {
		delete this;
	}
}

void WriteInt16(uchar_t *buf, int16_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void WriteUint16(uchar_t *buf, uint16_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void WriteInt32(uchar_t *buf, int32_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void WriteUint32(uchar_t *buf, uint32_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void WriteInt64(uchar_t *buf, int64_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 56);
	buf[1] = static_cast<uchar_t>((data >> 48) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 40) & 0xFF);
	buf[3] = static_cast<uchar_t>((data >> 32) & 0xFF);
	buf[4] = static_cast<uchar_t>((data >> 24) & 0xFF);
	buf[5] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[6] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[7] = static_cast<uchar_t>(data & 0xFF);
}

void WriteUint64(uchar_t *buf, uint64_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 56);
	buf[1] = static_cast<uchar_t>((data >> 48) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 40) & 0xFF);
	buf[3] = static_cast<uchar_t>((data >> 32) & 0xFF);
	buf[4] = static_cast<uchar_t>((data >> 24) & 0xFF);
	buf[5] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[6] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[7] = static_cast<uchar_t>(data & 0xFF);
}

int16_t ReadInt16(uchar_t *buf)
{
	int16_t data = (buf[0] << 8) | buf[1];
	return data;
}

uint16_t ReadUint16(uchar_t* buf)
{
	uint16_t data = (buf[0] << 8) | buf[1];
	return data;
}

int32_t ReadInt32(uchar_t *buf)
{
	int32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

uint32_t ReadUint32(uchar_t *buf)
{
	uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

int64_t ReadInt64(uchar_t *buf)
{
	int64_t data = ((int64_t) buf[0] << 56) | ((int64_t) buf[1] << 48) | ((int64_t) buf[2] << 40)
			| ((int64_t) buf[3] << 32) | ((int64_t) buf[4] << 24) | ((int64_t) buf[5] << 16) | ((int64_t) buf[6] << 8)
			| (int64_t) buf[7];
	return data;
}

uint64_t ReadUint64(uchar_t *buf)
{
	uint64_t data = ((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) | ((uint64_t) buf[2] << 40)
			| ((uint64_t) buf[3] << 32) | ((uint64_t) buf[4] << 24) | ((uint64_t) buf[5] << 16)
			| ((uint64_t) buf[6] << 8) | (uint64_t) buf[7];
	return data;
}

void show_hex(const char *data, int len, const char *str)
{
	unsigned char *d = (unsigned char*) data;
	if (str != NULL)
		printf("%s:", str);
	for (int i = 0; i < len; i++) {
		printf("%02x ", d[i]);
	}

	printf("\n");

	return;
}

