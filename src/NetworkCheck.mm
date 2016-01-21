/*****************************************************************************
 Name        : NetworkCheck.mm
 Author      : sotter
 Date        : 2015年6月9日
 Description :
 ******************************************************************************/

#import "NetworkCheck.h"
#import "netcheck.h"

@implementation NetworkCheck

+ (NSString*)netcheck:(NSString*)host port:(int)port data:(NSData *)data
{
    NetCheck nc;
    
    std::string result = nc.excute([host UTF8String], port, (const char*)[data bytes], (int)[data length]);
    
    NSString *str=[NSString stringWithUTF8String:result.c_str()];
    
    return str;
}

@end
