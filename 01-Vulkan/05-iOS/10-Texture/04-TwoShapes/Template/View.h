#import <UIKit/UIKit.h>

@interface View:UIView<UIGestureRecognizerDelegate>

-(void) startDisplayLink;
-(void) stopDisplayLink;

@end
