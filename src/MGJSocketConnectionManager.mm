

#import "MGJSocketConnectionManager.h"
#import "network.h"


@interface MGJSocketConnection ()
@property (nonatomic) NSString *serverIP;
@property (nonatomic) NSInteger port;
@property (nonatomic, weak) MGJSocketConnectionManager *socketManager;
@property (nonatomic, assign) int connectionID;
@property (nonatomic, assign) MGJSocketConnectionStatus status;
@property (nonatomic, assign) BOOL encrypt;
@end


class IOSNetWork : public cppnetwork::NetWork {
    
public:
    void setIosService(MGJSocketConnectionManager *s);
    virtual void onConnect(int handle);
    virtual void onRead(int handle, const char *buf, int len);
    virtual void onClose(int handle, int reason);
    
private:
    __weak MGJSocketConnectionManager *_oc_network_service;
};


//使用继承IOS
@interface MGJSocketConnectionManager()
{
    IOSNetWork *_service;
}

@property (nonatomic, strong) NSMutableDictionary *connections;
- (BOOL)connect:(MGJSocketConnection *)connection;

- (void)close:(int)connectionID;

- (BOOL)sendData:(NSData *)data withConnection:(int)connectionID;

- (MGJSocketConnectionStatus)getStatusForConnection:(int)connectionID;

- (void)onConnect:(int)connectionID;

- (void)onRead:(int)connectionID data:(NSData *)data;

- (void)onClose:(int)connectionID reason:(int)reason;
@end



@implementation MGJSocketConnection
- (instancetype)init
{
    self = [super init];
    if (self) {
        _connectionID = -1;
    }
    return self;
}

- (BOOL)connect
{
    return [self.socketManager connect:self];
}

- (BOOL)sendData:(NSData *)data
{
    return [self.socketManager sendData:data withConnection:self.connectionID];
}

- (void)close
{
    [self.socketManager close:self.connectionID];
}

- (MGJSocketConnectionStatus)status
{
    return [self.socketManager getStatusForConnection:self.connectionID];
}
@end



#pragma mark - IOSNetWork

void IOSNetWork::setIosService(MGJSocketConnectionManager *s)
{
    _oc_network_service = s;
}

void IOSNetWork::onConnect(int handle)
{
    [_oc_network_service onConnect:handle];
}

void IOSNetWork::onRead(int handle, const char *buf, int len)
{
    NSData *pdu_data = [[NSData alloc] initWithBytes:buf length:len];
    [_oc_network_service onRead:handle data:pdu_data];
}

void IOSNetWork::onClose(int handle, int reason)
{
    [_oc_network_service onClose:handle reason:reason];
}


@implementation MGJSocketConnectionManager

+ (instancetype)sharedInstance
{
    static dispatch_once_t onceToken;
    static MGJSocketConnectionManager *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[MGJSocketConnectionManager alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    //init 中把_service中的_oc_network_service初始化
    self = [super init];
    if (self) {
        _service = new IOSNetWork();
        _service->init();
        _service->setIosService(self);
        _connections = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)dealloc
{
    delete _service;
}

#pragma mark - instance methods
- (MGJSocketConnection *)connectionWithServerIP:(NSString *)serverIP port:(NSInteger)port delegate:(id<MGJSocketConnectionDelegate>)delegate encrypt:(BOOL)encrypt
{
    NSParameterAssert(serverIP);
    
    MGJSocketConnection *connection = [[MGJSocketConnection alloc] init];
    connection.serverIP = serverIP;
    connection.port = port;
    connection.delegate = delegate;
    connection.socketManager = self;
    connection.encrypt = encrypt;
    return connection;
}

- (BOOL)connect:(MGJSocketConnection *)connection
{
    if (connection.connectionID != -1) {
        [self close:connection.connectionID];
        connection.connectionID = -1;
    }
    
    int connectionID = _service->connect([connection.serverIP UTF8String], connection.port,connection.encrypt);
    if (connectionID != -1) {
        connection.connectionID = connectionID;
        [self.connections setObject:connection forKey:@(connectionID)];
        return YES;
    }
    else {
        return NO;
    }
}

- (void)close:(int)connectionID
{
    _service->close(connectionID);
    [self.connections removeObjectForKey:@(connectionID)];
}

- (BOOL)sendData:(NSData *)data withConnection:(int)connectionID
{
    const char *buf = (const char*)[data bytes];
    return _service->send(connectionID, buf, (int)[data length]);
}

- (MGJSocketConnectionStatus)getStatusForConnection:(int)connectionID
{
    MGJSocketConnection *connection = self.connections[@(connectionID)];
    if (connection) {
        return (MGJSocketConnectionStatus)(_service->getStatus(connectionID));
    }
    else {
        return MGJSocketConnectionStatusClosed;
    }
}

#pragma mark - delegate

- (void)onConnect:(int)connectionID
{
    MGJSocketConnection *connection = self.connections[@(connectionID)];
    if ([connection.delegate respondsToSelector:@selector(connectionDidConnect:)]) {
        [connection.delegate connectionDidConnect:connection];
    }
}

- (void)onClose:(int)connectionID reason:(int)reason
{
    MGJSocketConnection *connection = self.connections[@(connectionID)];
    if ([connection.delegate respondsToSelector:@selector(connection:didCloseForReason:)]) {
        [connection.delegate connection:connection didCloseForReason:(MGJSocketConnectionCloseReason)reason];
    }
    [self.connections removeObjectForKey:@(connectionID)]; 
}

- (void)onRead:(int)connectionID data:(NSData *)data
{
    MGJSocketConnection *connection = self.connections[@(connectionID)];
    if ([connection.delegate respondsToSelector:@selector(connection:didReceiveData:)]) {
        [connection.delegate connection:connection didReceiveData:data];
    }
}
@end
