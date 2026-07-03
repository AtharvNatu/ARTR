#import "SceneDelegate.h"
#import "ViewController.h"
#import "View.h"

@implementation SceneDelegate
{
    @private
    UIWindow* window;
    View* view;
}

-(void) scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions
{
    // Variable Declarations
    UIWindowScene* windowScene = nil;
    ViewController* viewController = nil;
    view = nil;
    CGRect rect;
    
    // Code
    
    // Verify that the provided scene is of type UIWindowScene
    if ([scene isKindOfClass:[UIWindowScene class]] == NO)
        return;
    
    // Cast and store the scene as a UIWindowScene instance
    windowScene = (UIWindowScene*)scene;
    
    // Create and initialize the application window with the scene
    window = [[UIWindow alloc]initWithWindowScene:windowScene];
    
    // Set the window's background color to black
    [window setBackgroundColor:[UIColor blackColor]];
    
    // Create our custom view controller which will work as main view controller for all views - Root View Controller
    viewController = [[ViewController alloc]init];
    
    // Set viewController as our Window's Root View Controller
    [window setRootViewController:viewController];
    
    // Window increments the view controller's reference count when we assign it as the Root View Controller. To adjust this count, call release
    [viewController release];
    
    // Create a custom view with dimensions equal to the scene's bounds
    rect = [[[windowScene effectiveGeometry] coordinateSpace] bounds];
    
    view = [[View alloc]initWithFrame:rect];
    if ([view isKindOfClass:([View class])])
        [view startDisplayLink];
    
    // Set this view as the view controller's view
    [viewController setView:view];
    
    // ViewController increments the view's reference count. To adjust this count, call release
    [view release];
    
    // Set focus and visibility
    [window makeKeyAndVisible];
}

-(void) sceneDidDisconnect:(UIScene *)scene
{
    // Code
    if ([view isKindOfClass:([View class])])
        [view stopDisplayLink];
}

-(void) sceneDidBecomeActive:(UIScene *)scene
{
    // Code
    if ([view isKindOfClass:([View class])])
        [view startDisplayLink];
}

-(void) sceneWillResignActive:(UIScene *)scene
{
    // Code
    if ([view isKindOfClass:([View class])])
        [view stopDisplayLink];
}

-(void) sceneWillEnterForeground:(UIScene *)scene
{
    // Code
}

-(void) sceneDidEnterBackground:(UIScene *)scene
{
    // Code
}

-(void) dealloc
{
    // Code
    [window release];
    [super dealloc];
}


@end
