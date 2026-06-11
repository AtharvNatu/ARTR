// Header Files
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>    // For CoreVideo
#import <QuartzCore/CAMetalLayer.h>     // Metal-Based Core Animation Layer

// Macros
#define WIN_WIDTH   800
#define WIN_HEIGHT  600

// Global Variable Declarations
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

bool bActiveWindow = NO;
bool bFullscreen = NO;
bool bWindowMinimized = NO;

char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// Global Function Declarations
CVReturn DisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*, CVOptionFlags, CVOptionFlags*, void*);

// Forward Interface Declarations
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface View : NSView <NSWindowDelegate>
@end

// Entry Point Function
int main(int argc, char* argv[])
{
    // Code

    //* Create AutoreleasePool For Reference Counting
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

    //* Create Application Object For This Application
    NSApp = [NSApplication sharedApplication];

    //* Set Activation Policy
    [NSApp setActivationPolicy:(NSApplicationActivationPolicyRegular)];

    //* Set AppDelegate Object For NSApp
    [NSApp setDelegate:[[AppDelegate alloc]init]];

    //* Start Game Loop
    [NSApp run];

    //* Tell AutoreleasePool To Release All Objects Created By This Application
    [pool release];

    return 0;
}

// AppDelegate Interface Implementation
@implementation AppDelegate
{
    @private
    NSWindow *window;
    View *view;
}

-(void) applicationDidFinishLaunching:(NSNotification *)notification 
{
    // Code
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirectoryName = [appBundle bundlePath];
    NSString *parentDirectoryPath = [appDirectoryName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/%s", parentDirectoryPath, gszLogFileName];
    const char* pszLogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];

    // Log File
    gpFile = fopen(pszLogFileNameWithPath, "w");
    if (gpFile == NULL)
    {
        NSLog(@"Failed To Create Log File ... Exiting !!!");
        [self release];
        [NSApp terminate:self];
        return;
    }
    else 
        fprintf(gpFile, "Program Started Successfully");

    // Create Window
    NSRect winRect = NSMakeRect(0.0, 0.0, WIN_WIDTH, WIN_HEIGHT);

    window = [[NSWindow alloc]initWithContentRect: winRect
                              styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                              backing: NSBackingStoreBuffered
                              defer: NO];

    [window setTitle:@"Atharv Natu : macOS Window Template For MoltenVk"];
    [window setBackgroundColor:[NSColor blackColor]];
    [window center];

    // Create View
    view = [[View alloc]initWithFrame:winRect];

    // Set The Created View As The Content View Of Our Window
    [window setContentView:view];

    // Set App Icon
    NSString *iconFilePath = [[NSBundle mainBundle] pathForResource:@"Arch" ofType:@"png"];

    NSImage *icon = [[NSImage alloc]initWithContentsOfFile:iconFilePath];
    if (icon)
    {
        // Runtime Icon -> For Dock
        [NSApp setApplicationIconImage:icon];

        // App Icon -> For Finder
        [[NSWorkspace sharedWorkspace]setIcon:icon 
                                    forFile:appDirectoryName 
                                    options:0];
        [icon release];
        icon = nil;
    }
    else
        fprintf(gpFile, "\nFailed To Set App Icon !!!");

    // Set Window's Delegate To This Object
    [window setDelegate:view];
    [window makeKeyAndOrderFront:self];

    // Tell NSApp To Activate This Window And Ignore Other Window
    [NSApp activateIgnoringOtherApps:YES];
}

-(void) applicationWillTerminate:(NSNotification *)notification 
{
    // Code
    [self release];
}

-(void) dealloc 
{
    // Code
    if (view)
    {
        [view release];
        view = nil;
    }

    if (window)
    {
        [window release];
        window = nil;
    }

    [super dealloc];
}

@end


// View Interface Implementation
@implementation View
{
    @private
    CVDisplayLinkRef displayLink;
}

-(id) initWithFrame:(NSRect)frame 
{
    // Code
    self = [super initWithFrame:frame];
    if (self)
    {
        //* Transform Our View To CAMetalLayer Backing View (NSView -> Metal Backing View)
        [self setWantsLayer:YES];

        int result = [self initialize];
        if (result != 0)
            fprintf(gpFile, "\n%s() => initialize() Failed : %d !!!\n", __func__, result);
        else
            fprintf(gpFile, "\n%s() => initialize() Succeeded\n", __func__);

        //* Create A Display Link Capable Of Being Used By All Active Displays
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

        //* Set The Display Link As Our Rendering Output Callback
        CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, self);

        //* Activate The Display Link
        CVDisplayLinkStart(displayLink);
    }

    return self;
}

-(void) windowDidBecomeKey:(NSNotification *)notification 
{
    // Code
    bActiveWindow = YES;
}

-(void) windowDidResignKey:(NSNotification *)notification 
{
    // Code
    bActiveWindow = NO;
}

-(NSSize) windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize 
{
    // Code
    CVDisplayLinkStop(displayLink);

    if (bWindowMinimized == NO)
    {
        [self resize: frameSize.width :frameSize.height];
    }

    return frameSize;
}

-(void) windowDidResize:(NSNotification *)notification 
{
    // Code
    CVDisplayLinkStart(displayLink);
}

-(void) windowWillMiniaturize:(NSNotification *)notification 
{
    // Code
    bWindowMinimized = YES;
    CVDisplayLinkStop(displayLink);
}

-(void) windowDidMiniaturize:(NSNotification *)notification 
{
    // Code - Empty Body -> Written For Delegate Implementation
}

-(void) windowDidDeminiaturize:(NSNotification *)notification 
{
    // Code
    bWindowMinimized = NO;
    CVDisplayLinkStart(displayLink);
}

-(void) windowWillClose:(NSNotification *)notification 
{
    // Code
    [self uninitialize];
    [NSApp terminate:self];
}

-(BOOL) acceptsFirstResponder
{
    // Code
    [[self window]makeFirstResponder:self];
    return YES;
}

-(void) keyDown:(NSEvent *)event
{
    // Code
    int key = (int)[[event characters]characterAtIndex:0];
    switch(key)
    {
        case 27:
            if (bFullscreen)
            {
                [[self window]toggleFullScreen:nil];
                bFullscreen = NO;
            }
            [[self window]performClose:self];
        break;

        case 'F':
        case 'f':
            if (bFullscreen == NO)
            {
                [[self window]toggleFullScreen:self];
                bFullscreen = YES;
            }
            else 
            {
                [[self window]toggleFullScreen:self];
                bFullscreen = NO;
            }  
        break;

        default:
        break;
    }
}

-(int) initialize
{
    // Code
    return 0;
}

-(void) resize:(int)width :(int)height
{
    // Code
}

-(void) drawRect:(NSRect)dirtyRect
{
    // Code

    //* To Prevent Flickering (Image Tearing) Happening Due To Synchronization Issues - Call Rendering Function
    [self drawView];
}

-(CVReturn) getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    // Code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

    //* Render The Scene
    [self drawView];

    [pool release];

    return kCVReturnSuccess;
}

//! For setWantsLayer() To Return True We Must Override/Implement The Following 2 Static Functions
//!     1) Needed For Draw
//!     2) Needed For Update

+(Class) layerClass
{
    // Code
    return [CAMetalLayer class];
}

//* Continuosly Demand The Updated Layer, Which Is Updated By Rendering
-(BOOL) wantsUpdateLayer 
{
    // Code
    return YES;
}

//* To Have The Result Of setWantsLayer, The Following Function Needs To Return Resized Layer, If Resizing Is Done
-(CALayer*) makeBackingLayer 
{
    // Code
    CALayer* layer = [[[self class]layerClass]layer];

    CGSize viewSize = [self convertSizeToBacking:CGSizeMake(1.0, 1.0)];

    [layer setContentsScale:MIN(viewSize.width, viewSize.height)];

    return layer;
}


-(void) drawView
{
    // Code
    [self display];

    [self update];
}

-(void) display
{
    // Code
}

-(void) update
{
    // Code
}

-(void) uninitialize
{
    // Code
    if (bFullscreen)
    {
        [[self window]toggleFullScreen:nil];
        bFullscreen = NO;
    }

    if (gpFile)
    {
        fprintf(gpFile, "Program Terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void) dealloc 
{
    // Code
    if (displayLink)
    {
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        displayLink = NULL;
    }

    [super dealloc];
}

@end

// Callback Implementation
CVReturn DisplayLinkCallback(
    CVDisplayLinkRef displayLinkRef, 
    const CVTimeStamp* now, 
    const CVTimeStamp* outputTime, 
    CVOptionFlags flagsIn, 
    CVOptionFlags* flagsOut, 
    void* renderer)
{
    // Code
    CVReturn result = [(View*)renderer getFrameForTime:outputTime];

    return result;
}

