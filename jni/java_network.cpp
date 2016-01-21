

#include "network.h"
#include "netcheck.h"
#include "java_network.h"
#include "log.h"

#include <jni.h>

#ifdef ANDROID
//全局变量
JavaVM *g_jvm = NULL;
jobject g_obj = NULL;
#endif

using namespace cppnetwork;

class AndroidNetwork : public cppnetwork::NetWork
{
public:

	AndroidNetwork(){}

	virtual ~AndroidNetwork(){}

	//连接结果通知
    virtual void onConnect(int handle)
    {
    	//todo:其实可以直接在这里面实现JNI的接口
    	::onConnect(handle);
    }

    //数据到来通知，交付出来的已经是一个完整的PDU包
    virtual void onRead(int handle, const char* buffer, int len)
    {
    	::onRead(handle, buffer, len);
    }

    //网络断开数据通知，reason：失败处理的缘由
    virtual void onClose(int handle, int reason)
    {
    	::onClose(handle, reason);
    }
};

static AndroidNetwork g_network;

static char* jstringTostr(JNIEnv* env, jstring jstr)
{
	char* pStr = NULL;

	jclass jstrObj = env->FindClass("java/lang/String");
	jstring encode = env->NewStringUTF("utf-8");
	jmethodID methodId = env->GetMethodID(jstrObj, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray byteArray = (jbyteArray) env->CallObjectMethod(jstr, methodId, encode);
	jsize strLen = env->GetArrayLength(byteArray);
	jbyte *jBuf = env->GetByteArrayElements(byteArray, JNI_FALSE);

	if (jBuf > 0)
	{
		pStr = (char*) malloc(strLen + 1);

		if (!pStr)
		{
			return NULL;
		}

		memcpy(pStr, jBuf, strLen);

		pStr[strLen] = 0;
	}

	env->ReleaseByteArrayElements(byteArray, jBuf, 0);

	return pStr;
}

JNIEXPORT void JNICALL Java_com_mogujie_io_Network_initEnv(JNIEnv* env, jobject thiz)
{
	env->GetJavaVM(&g_jvm);
	g_obj = env->NewGlobalRef(thiz);
}

//todo 系统启动就会触发这个，为什么不能链接建立成功之后触发。
JNIEXPORT void JNICALL Java_com_mogujie_io_Network_runEvent(JNIEnv* env, jclass thiz)
{
	g_network.init();
}

JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_getStatus(JNIEnv* env, jclass thiz, jint handle)
{
	return g_network.getStatus(handle);
}

JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_connect(JNIEnv* env, jclass thiz, jstring ip, jint port, jint encrypt)
{
	char* pIp = jstringTostr(env, ip);

	if(encrypt == 0)
	{
		return g_network.connect(pIp, port, false);
	}
	else
	{
		return g_network.connect(pIp, port, true);
	}
}

JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_write(JNIEnv* env, jclass thiz, jint handle, jbyteArray msg, jint len)
{
	jint ret = 0;

	jboolean isCopy;
	jbyte* pData = env->GetByteArrayElements(msg, &isCopy);

	if(!g_network.send(handle, (const char*)pData, len))
	{
		ret = -1;
	}

	env->ReleaseByteArrayElements(msg, pData, 0);

	//todo:关注下返回结果
	return ret;
}

JNIEXPORT void JNICALL Java_com_mogujie_io_Network_close(JNIEnv* env, jclass thiz, jint handle)
{
	g_network.close(handle);

	jclass cls = env->GetObjectClass(thiz);
	jmethodID mid = env->GetStaticMethodID(cls, "onClose", "(II)V");
	if (NULL == mid) {
		LOGE("GetMethodID() Error.....");
		return;
	}
	env->CallStaticVoidMethod(cls, mid, handle, SOCKET_ERROR_CLOSE);
}

JNIEXPORT jbyteArray JNICALL Java_com_mogujie_io_Network_netCheck(JNIEnv* env, jclass thiz, jstring host,
		jint port, jbyteArray msg, jint len)
{
	LOGE("into Java_com_mogujie_io_Network_netCheck");

	NetCheck nc;
	NetCheckResult ncret;
	const char *nchost = jstringTostr(env, host);
	unsigned short ncport = (unsigned short) (port);
	jboolean isCopy;
	jbyte* ncmsg = env->GetByteArrayElements(msg, &isCopy);

	string json_result = nc.excute(nchost, ncport, (const char*)ncmsg, len);

	int result_len = (int)json_result.length();
	jbyteArray carr = env->NewByteArray(result_len);
	env->SetByteArrayRegion(carr, 0, result_len, (jbyte*)json_result.c_str());

	return carr;
}

// 以下代码的编码风格，保持原样，不做修改了
void onRead(int nHandle, const char* pBuf, int nLen)
{
	JNIEnv *env;
	jclass cls;
	jmethodID mid;

	if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}

	cls = env->GetObjectClass(g_obj);
	if (NULL == cls) {
		LOGE("FindClass() Error.....");

		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}

	mid = env->GetStaticMethodID(cls, "onRead", "(I[BI)V");
	if (NULL == mid) {
		LOGE("GetMethodID() Error.....");
		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}

	jbyteArray carr = env->NewByteArray(nLen);
	env->SetByteArrayRegion(carr, 0, nLen, (jbyte*) pBuf);
	env->CallStaticVoidMethod(cls, mid, nHandle, carr, nLen);
	if (g_jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}

void onClose(int nHandle, int nReason)
{
	JNIEnv *env;
	jclass cls;
	jmethodID mid;
	if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}
	cls = env->GetObjectClass(g_obj);
	if (NULL == cls) {
		LOGE("FindClass() Error.....");

		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}
	mid = env->GetStaticMethodID(cls, "onClose", "(II)V");
	if (NULL == mid) {
		LOGE("GetMethodID() Error.....");
		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}
	env->CallStaticVoidMethod(cls, mid, nHandle, nReason);
	if (g_jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}

void onConnect(int nHandle)
{
	JNIEnv *env;
	jclass cls;
	jmethodID mid;

	if (!g_jvm) {
		LOGE("jvm is null");
	}
	if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}
	cls = env->GetObjectClass(g_obj);
	if (NULL == cls) {
		LOGE("FindClass() Error.....");

		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}
	mid = env->GetStaticMethodID(cls, "onConnect", "(I)V");
	if (NULL == mid) {
		LOGE("GetMethodID() Error.....");
		//Detach主线程
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
		return;
	}
	env->CallStaticVoidMethod(cls, mid, nHandle);
	if (g_jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;
	//获取JNI版本
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("GetEnv failed!");
		return result;
	}
	return JNI_VERSION_1_4;
}
