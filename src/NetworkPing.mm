/*****************************************************************************
 Name        : NetworPing.mm
 Author      : sotter
 Date        : 2015年6月23日
 Description :
 ******************************************************************************/

#import "NetworkPing.h"
#import "ping.h"


@implementation NetworkPing

+ (NSString*)netping:(NSString*)host
{
    cppnetwork::Ping nc;
    
    std::string result = nc.ping([host UTF8String]);
    
    NSString *str=[NSString stringWithUTF8String:result.c_str()];
    
    return str;
}

@end
