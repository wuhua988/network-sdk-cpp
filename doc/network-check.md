###网络监测接口使用说明

#####1、接口类型原型

- IOS

	``` +(NSString*)netcheck:(NSString*)host port:(int)port data:(NSData*)data; ```

- Android

	``` public native byte[] netCheck(String host, int port, byte[]message, int len);```

- 参数说明

	host: 监测的目标服务器地址，可以是IP或是域名
	
	port: 监测端口
	
	data/message: 传输监测的数据，可以根据实际的业务需求发送不同内容的数据流；
	
	len：监测数据长度（for android）
	
	返回值：内容为json字符串的字节流
	
#####2、JSON结果说明

- 示例

```

{"parse_dns_result":-1,"parse_dns_use_time":0,"connect_result":6,"connect_time":0,"send_req_result":-1,"send_req_bytes":0,"send_recv_time":0,"recv_rsp_result":-1,"recv_rsp_bytes":0,"error":111,"errstr":"Connection refused"}
```

- result类结果
  
  包括（parse_dns_result, connect_result, send_req_result, recv_rsp_result
    
** 值定义如下: **
  
```
//没有运行到那一步
NORUN                    (-1)

//运行成功
OK                       (0)

//远端关闭了连接
SOCK_REMOTE_CLOSE        (1)

//创建socketerror
SOCK_ERROR               (2)

//解析DNS失败
PARSE_DNS_FAIL           (4)

//与服务端建立连接超时
SOCK_CONN_TIMEOUT        (5)

//监测到建立连接失败，此时连接失败的错误码放在error和errstr中
SOCK_CONN_FAIL           (6)

//send送入的参数错
SOCK_SEND_PARAM_ERROR    (7)

//连接不存在或已经断开
SOCK_SEND_CONN_ERROR     (8)

//send时发生系统错误（select调用失败），错误码放入到errno中
SOCK_SEND_SYS_ERROR      (9)

//发送超时
SOCK_SEND_TIMEOUT        (10)

//读取时传递的参数错误
SOCK_RECV_PARAM_ERROR    (11)

//连接不存在或已经断开
SOCK_RECV_CONN_ERROR     (12)

//读取数据超时，注意：这里的超时指的是没有收到任何数据，不是收到数据不全
SOCK_RECV_TIMEOUT        (13)

//读取数据时发生系统错误，错误码放在Record中的errno中
SOCK_RECV_SYS_ERROR      (14)

```

- 消耗时间

parse_dns_use_time : 解析DNS花费的时间 （只有存在DNS解析时此值才有效）

connect_time       : 建立连接花费的时间（只有连接成功时此值才有效）

send_recv_time     : 发送消息和接受到相应之间的时间（只有发送和接收都成功的时候此值才有效）


- 错误码/错误字符串
  
  error保存了运行中系统的errno值，要跟此时的运行阶段判定属于哪一个阶段产生的errno，errstr是根据errno产生的错误字符串；
