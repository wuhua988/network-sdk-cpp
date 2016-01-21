package com.mogujie.io;

import android.util.Log;

public class Network {
    static {
        System.loadLibrary("network");
    }

    private static Network m_pInstance;

    public static Network getInstance() {
        synchronized (Network.class) {
            if (m_pInstance == null) {
                m_pInstance = new Network();
            }

            return m_pInstance;
        }
    }

    private Network() {
        Log.i("Network", "Network");
        initEnv();
    }

    public native void initEnv();

    public native void runEvent();
    
    //0: 非加密 1: 加密
    public native int connect(String ip, int port, int encrypt);

    public native int getStatus(int handle);

    public native int write(int handle, byte[] message, int len);

    public native void close(int handle);
    
    public native byte[] netCheck(String host, int port, byte[]message, int len);
    
    public static void onRead(int handle, byte[] message, int len) {
        Log.i("Network", "onRead:" + String.valueOf(handle));
    }

    /*
    enum
    {
        SOCKET_ERROR_TIMEOUT = 1,超时
        SOCKET_ERROR_READ = 2,读错误
        SOCKET_ERROR_WRITE = 3,写错误
        SOCKET_ERROR_EXCEPTION = 4,异常
        SOCKET_ERROR_CLOSE = 5,主动关闭
        SOCKET_ERROR_SYS = 6,系统错误
    };
    */
    public static void onClose(int handle, int reason) {
        Log.i("Network", "onClose:" + String.valueOf(handle) + ";reason:" + String.valueOf(reason));
    }


    /*
     enum
     {
     SOCKET_STATE_IDLE = 1,
     SOCKET_STATE_CONNECTING = 2,
     SOCKET_STATE_CONNECTED = 3,
     SOCKET_STATE_CLOSING = 4,
     };
     */
    public static void onConnect(int handle) {
        Log.i("Network", "onConnect:" + String.valueOf(handle));
    }
}
