
#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, MGJSocketConnectionStatus)
{MGJSocketConnectionStatusConnected = 0x00,
MGJSocketConnectionStatusClosed = 0x01,
MGJSocketConnectionStatusConnecting = 0x02,
MGJSocketConnectionStatusInvalidHandle = 0x03,
};

typedef NS_ENUM(NSUInteger, MGJSocketConnectionCloseReason)
{MGJSocketConnectionCloseReasonTimeout = 1,
MGJSocketConnectionCloseReasonConnecting = 3,
MGJSocketConnectionCloseReasonException = 4,
MGJSocketConnectionCloseReasonClose = 5,
MGJSocketConnectionCloseReasonSys = 6,
};

@class MGJSocketConnection;
@protocol MGJSocketConnectionDelegate<NSObject>

- (void)connectionDidConnect:(MGJSocketConnection *)connection;
- (void)connection:(MGJSocketConnection *)connection didCloseForReason:(MGJSocketConnectionCloseReason)reason;
- (void)connection:(MGJSocketConnection *)connection didReceiveData:(NSData *)data;
@end

@interface MGJSocketConnection : NSObject
@property (nonatomic, weak) id<MGJSocketConnectionDelegate> delegate;
@property (nonatomic, readonly) NSString *serverIP;
@property (nonatomic, readonly) NSInteger port;
@property (nonatomic, readonly, assign) int connectionID;
@property (nonatomic, readonly, assign) BOOL encrypt;
@property (nonatomic, readonly, assign) MGJSocketConnectionStatus status;

- (BOOL)connect;

- (void)close;

- (BOOL)sendData:(NSData *)data;

@end

@interface MGJSocketConnectionManager : NSObject

+ (instancetype)sharedInstance;

- (MGJSocketConnection *)connectionWithServerIP:(NSString *)serverIP port:(NSInteger)port delegate:(id<MGJSocketConnectionDelegate>)delegate encrypt:(BOOL)encrypt;

@end

