#import "ViewController.h"

@implementation ViewController

-(void) viewDidLoad
{
    // Code
    [super viewDidLoad];
}

// Following code will assist the setting done in General Settings for landscape mode
-(UIInterfaceOrientationMask) supportedInterfaceOrientations
{
    // Code
    return UIInterfaceOrientationMaskLandscape;
}

// Hide Status Bar - Assist the status bar setting done in General Settings
-(BOOL) prefersStatusBarHidden
{
    // Code
    return YES;
}

-(void) dealloc
{
    // Code
    [super dealloc];
}


@end
