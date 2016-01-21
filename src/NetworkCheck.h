/*****************************************************************************
 Name        : NetWorkCheck.h
 Author      : sotter
 Date        : 2015年6月9日
 Description :
 ******************************************************************************/

#import <Foundation/Foundation.h>

/*返回结果注释：
 返回的结果为JSON格式，示例如下：
 {"parse_dns_result":-1,"server_host":"10.0.30.125","server_port":4312,"parse_dns_use_time":0,"connect_result":6,"connect_time":0,"send_req_result":-1,"send_req_bytes":0,"recv_rsp_result":-1,"recv_rsp_bytes":0,"send_recv_time":0,"error":111,"errstr":"Connection refused"}

 parse
 server_host：        表示解析出来的DNS地址，如果不是域名格式，返回为输入的IP地址；
 server_port：        返回输入端口号
 parse_dns_use_time:  解析域名花费时间
 connect_time：       与服务端建立连接话费的时间
 send_recv_time：     从发送请求到接收到回复花费的时间

 parse_dns_result, connect_result, send_req_result, recv_rsp_result的定义如下：

 //没有运行到那一步
 #define NORUN                    (-1)

 //运行成功
 #define OK                       (0)

 #define SOCK_REMOTE_CLOSE        (1)
 #define SOCK_ERROR               (2)

 #define PARSE_DNS_FAIL           (4)
 //连接超时
 #define SOCK_CONN_TIMEOUT        (5)
 //此时具体原因放入到errno
 #define SOCK_CONN_FAIL           (6)
 //send送入的参数错误
 #define SOCK_SEND_PARAM_ERROR    (7)
 //连接不存在或已经断开
 #define SOCK_SEND_CONN_ERROR     (8)
 //select发生错误，错误码放入到errno中
 #define SOCK_SEND_SYS_ERROR      (9)
 //发送超时
 #define SOCK_SEND_TIMEOUT        (10)
 //读取时传递的参数错误
 #define SOCK_RECV_PARAM_ERROR    (11)
 //连接不存在或已经断开
 #define SOCK_RECV_CONN_ERROR     (12)
 //读取数据超时，注意：这里的超时指的是没有收到任何数据，不是收到数据不全
 #define SOCK_RECV_TIMEOUT        (13)
 //读取数据时发生系统错误，错误码放在Record中的errno中
 #define SOCK_RECV_SYS_ERROR      (14)

 */

@interface NetworkCheck : NSObject

+(NSString*)netcheck:(NSString*)host port:(int)port data:(NSData*)data;

@end
