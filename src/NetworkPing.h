/*****************************************************************************
 Name        : NetWorkCheck.h
 Author      : sotter
 Date        : 2015年6月9日
 Description :
 ******************************************************************************/

#import <Foundation/Foundation.h>

@interface NetworkPing : NSObject

+(NSString*)netping:(NSString*)host;

@end
