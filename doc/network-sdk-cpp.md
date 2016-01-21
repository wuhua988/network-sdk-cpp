## <center> libnetwork(cppnetwork使用说明) </center> ###
=======================
tianshan 2015-06-18

1. 代码路径：（目前还没有合并到主分支，在V1.1分支上获取，待初步测试后会合并到master分支）
   <http://gitlab.mogujie.org/im/libnetwork.git>
   
   checkout v1.1 
   
2. 编译
   android： libnetwork-1.0目录下执行ndk-build，编译好的库放在libs下面；
   IOS    ： 直接导入代码的方式，需要注意的是编译时需要依赖IOS目录静态库
   
3. 接口修改：
   注意：connect接口最后增加了一个参数，encrypt是否加密，1：加密，0：表示未加密；如果设置了加密，网络库与自动与服务端发起加密会话，待加密会话完成后，才回onConnect事件；
   
5. 测试服务端
   access-server：221.228.199.52 38000
   
4. 特别注意：
   需要在各种测试机下验证openssl库的兼容性
   
```
V1.1 change log 2015-07-16

（1）统一了android和IOS的网络库代码;
	* 原先android的网络库统一使用IOS的网络库代码
	* 代码目录结构发生了改变:
      cppnetwork->存放cpp源码和oc接口源码
	  android---->存放android依赖的第三方库
	  ios-------->存放ios依赖的第三方库
	  java------->存放封装的java源码
	  jni-------->存放android jni源代码
	  libs------->ndk-build 产生的libs文件的默认存放路径
	  test------->测试代码
	
（2）增加了超时管理能：
	* 如果在10分钟内client与server之间没有任何数据交互，将回收handle所占用的资源；
	* 如果5秒内未完网络连接或是加密会话，认为操作超时，收回handle资源；
	
（3）增加了加密功能：
	* 加密粒度针对每一条连接；connect调用增加一个是否加密的参数；
	* 如果在connect的时候设置了加密，那么在网络连接成功后，会主动发起与服务端的会话，双方交换密钥，待会话结束后才开始调用onConnect；整个会话过程对应用无感知；

注意事项：
（1） 因为移植了OPENSSL库，请充分验证在各种机型和模拟器条件下lib库的兼容性；

openssl库在移动端下面的编译需要修改，引用一下github连接，感谢开源！
https://github.com/x2on/OpenSSL-for-iPhone.git
https://github.com/armatys/openssl-android.git

```

### 一、目录结构说明

清除原先基于一个android工程构建的目录结构, 合并IOS和ANDROID的cpp代码，使之公用一份；

最上层目录libnetwork-1.0

+ ***cppnetwork***
  : 存放C++库源码，同时包含objective-c的封装接口；
  
+ ***doc***
  : 存放帮助文档
  
+ ***java***
  : 存放封装的java接口
  
+ ***jni***
  : 存放android jni源代码
  
+ ***libs***
  : 存放ndk-build交叉编译后库文件
  
+ ***obj***
  : ndk-build产生的中间文件放目录

### 二、使用说明

+ #### 2.1 demo程序测试（同时支持os x和 linux 的编译运行）
    - 在cppnetwork下执行 make
	- 运行cppnetwork下的test程序
	
+ #### 2.2 IOS使用
   
   - 直接将cppnetwork目录中的代码引入到oc工程中即可使用
   - 封装好的OC接口放在NetWorkService.h 和 NetworkCheck.h中

+ #### 2.3 ANDROID使用
   - 在根目录（libnetwork-1.0）下执行 ndk-build
   - 将libs的文件拷贝到需要引用的目录中，或者直接将libs目录引入到java.library.path中
   - 接口文件在java/com/mogujie/io/Network.java中


### 三、接口说明

- #### 3.1 初始化

	``` 
	Android： 
	// 初始化JNI环境
	public native void initEnv();
	// 启动异步IO线程
    public native void runEvent()
	```
	```
	// 构造函数中已经完成初始化
	IOS： -(instancetype)init;
	```

- #### 3.2 发起连接
	
	```
	 Android： public native int connect(String ip, int port, int encrypt); </code>
	```
	```
	IOS： -(int)connect:(NSString*)serverIP serverPort:(int)serverPort;
	```
	
	<p>说明:</p>
	（1）return：连接唯一标识handle，handle > 0 成功；han想·dle < 0 失败（获取handle失败，非连接失败);
	
	（2）此接口为异步接口，具体连接结果在onConnect和onClose判定；
	
    （3）network中已经增加超时管理功能，如果在10分钟与服务端没有任何交互，自动关闭连接，释放资源；
	
	
- #### 3.3 连接成功通知

	``` 
	Android： public static void onConnect(int handle)
	```
	```
	IOS： (void)server:(NetWorkService *)service onConnect:(int)handle;
	```
	说明：连接成功通知，编制handle连接成功；
	
- #### 3.4 获取连接状态通知

	``` 
    Android： public native int getStatus(int handle);

	```
	```
	IOS： -(int)getStatus:(int)handle;
	```
	说明：获取当前handle的连接状态：
	<p>  NET_STATE_CONNECTED      (0x00)  连接状态 </p>
	<p>  NET_STATE_CLOSED         (0x01)  断连状态 </p>
	<p>  NET_STATE_CONNECTING     (0x02)  异步连接正在连接状态 </p>
	<p>  NET_STATE_INVALID_HANDLE (0x03)  查询不到此handle，一般handle资源释放后再返回会返回此结果 </p>
	
- #### 3.5 关闭连接

	``` 
    Android： public native void close(int handle);

	```
	```
	IOS： -(void)close:(int)handle;
	```
	说明：关闭连接，***注意：在IOS版本及后续版本中，在旧android中会主动回调onClose 主动调用close不会有onClose通知产生。todo： 是否需要再回调通知给应用层待定（2015-06-18）***
	
- #### 3.6 发送数据

	``` 
    Android： public native int write(int handle, byte[] message, int len);

	```
	```
	IOS： -(int)send:(int)handle data:(NSData *)data;
	```
	说明：***此接口为异步接口，如果调用此接口后，再立即调用close可能会导致发送不成功，一定要注意！！！*** 返回0：投递到handle发送队列成功（非真实发送成功），-1：失败；
	
	
- #### 3.7 接收到数据通知

	``` 
    Android： public static void onRead(int handle, byte[] message, int len)
	```
	```
	IOS： (void)server:(NetWorkService *)service onRead:(int)handle data:(NSData*)data;
	```
	说明：通知到是一个完成的PDU包
	
- #### 3.8 连接断开通知

	``` 
	Android： public static void onClose(int handle, int reason) 
	```
	```
	IOS： (void)server:(NetWorkService *)service onClose:(int)handle reason:(int)reason;

	```
	
	说明：连接端开通知，reason含义如下：
	
	```
    SOCKET_ERROR_TIMEOUT = 1,     // 连接超时
	SOCKET_ERROR_READ             // 不再使用
	SOCKET_ERROR_WRITE            // 不再使用
    SOCKET_ERROR_CONN = 3,        // 连接失败，如对端端口没开
	SOCKET_ERROR_EXCEPTION = 4,   // 不再使用
	SOCKET_ERROR_CLOSE = 5,       // 运行过程中，发现网络断开
	SOCKET_ERROR_SYS = 6          // 不再使用
    ```
