
#ifndef __JAVA_NETWORK_H__
#define __JAVA_NETWORK_H__

#include <jni.h>

#ifdef  __cplusplus
extern "C" {
#endif
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);

    JNIEXPORT void JNICALL Java_com_mogujie_io_Network_initEnv(JNIEnv *, jobject);

    JNIEXPORT void JNICALL Java_com_mogujie_io_Network_runEvent(JNIEnv*, jclass);

    JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_getStatus(JNIEnv*, jclass, jint);

    JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_connect(JNIEnv*, jclass, jstring, jint, jint);

    JNIEXPORT jint JNICALL Java_com_mogujie_io_Network_write(JNIEnv* env, jclass, jint, jbyteArray msg, jint len);

    JNIEXPORT void JNICALL Java_com_mogujie_io_Network_close(JNIEnv*, jclass, jint);

    JNIEXPORT jbyteArray JNICALL Java_com_mogujie_io_Network_netCheck(JNIEnv*, jclass, jstring, jint, jbyteArray, jint);

    void onConnect(int nHandle);

    void onRead(int nHandle, const char* pBuf, int nLen);

    void onClose(int nHandle, int nReason);

#ifdef __cplusplus
}
#endif

#endif
