// Header Files
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

// Macros
#define WIN_WIDTH   800
#define WIN_HEIGHT  600

// Global Variable Declarations
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// Forward Interface Declarations
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface AppMTKView : MTKView <MTKViewDelegate>
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
    AppMTKView *view;
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

    [window setTitle:@"Atharv Natu : macOS Window Template For Metal"];
    [window setBackgroundColor:[NSColor blackColor]];
    [window center];

    // To Support Primary Monitor Fullscreen Between Multiple-Monitors
    [window setCollectionBehavior:(NSWindowCollectionBehaviorFullScreenPrimary)];

    // Create View
    view = [[AppMTKView alloc]initWithFrame:winRect];

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
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];

    // Tell NSApp To Activate This Window And Ignore Other Window
    [NSApp activateIgnoringOtherApps:YES];
}

-(void) applicationWillTerminate:(NSNotification *)notification 
{
    // Code
    if (gpFile)
    {
        fprintf(gpFile, "\nProgram Terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void) windowWillClose:(NSNotification *)notification 
{
    // Code
    [NSApp terminate:self];
}

-(void) dealloc 
{
    // Code
    [super dealloc];
}

@end

// View Interface Implementation
@implementation AppMTKView
{
    @private
    id<MTLDevice> metalDevice;
    id<MTLCommandQueue> metalCommandQueue;
    MTLViewport metalViewport;
    MTLClearColor metalClearColor;
}

-(id) initWithFrame:(NSRect)frame
{
    // Code
    self = [super initWithFrame:frame];
    if (self)
    {
        //* Set Content View
        [[self window]setContentView:self];

        //* Set Auto Resize
        [self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

        //* Set this view as its own delegate
        [self setDelegate:self];

        //* Initialize Metal
        [self initMetal];
    }

    return self;
}

-(void) initMetal
{
    // Code

    //* Create System Default Metal Device
    metalDevice = MTLCreateSystemDefaultDevice();

    //* Set above device as View's device
    [self setDevice:metalDevice];

    [self printMetalDeviceInfo];

    //* Create Command Queue
    metalCommandQueue = [metalDevice newCommandQueue];

    //* Set this View's pixel format
    [self setColorPixelFormat: MTLPixelFormatBGRA8Unorm];

    //* Set Clear Color
    metalClearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

    //* Warmup Resize
    [self mtkView:self drawableSizeWillChange:[self drawableSize]];
}

-(void) printMetalDeviceInfo
{
    fprintf(gpFile, "\n\nMETAL INFORMATION\n");
    fprintf(gpFile, "------------------------------------------------------------------------------------------------");
    fprintf(gpFile, "\nDevice Name                        : %s\n", [metalDevice.name UTF8String]);
    fprintf(gpFile, "Unified Memory                     : %s\n", metalDevice.hasUnifiedMemory ? "Yes (Apple Silicon/Integrated)" : "No (Discrete GPU)");
    fprintf(gpFile, "Recommended Mem Limit              : %llu MB (%llu GB)\n", metalDevice.recommendedMaxWorkingSetSize / (1024 * 1024), metalDevice.recommendedMaxWorkingSetSize / (1024 * 1024 * 1024));
    
    // Max threads per threadgroup (Compute/Compute Shaders pipeline capacity)
    MTLSize maxThreads = metalDevice.maxThreadsPerThreadgroup;
    fprintf(gpFile, "Max Threads/Group                  : [%lu, %lu, %lu]\n", maxThreads.width, maxThreads.height, maxThreads.depth);
    
    // Metal 3 Support Check (Modern Ray Tracing, Mesh Shaders)
    if (@available(macOS 13.0, *)) 
    {
        BOOL supportsMetal3 = [metalDevice supportsFamily:MTLGPUFamilyMetal3];
        fprintf(gpFile, "Supports Metal 3                   : %s\n", supportsMetal3 ? "Yes" : "No");
    }
    
    // Apple Silicon Generation Families Check
    if (@available(macOS 11.0, *)) 
    {
        fprintf(gpFile, "Apple GPU Family 8                 : %s\n", [metalDevice supportsFamily:MTLGPUFamilyApple8] ? "Yes" : "No");
        fprintf(gpFile, "Apple GPU Family 7                 : %s\n", [metalDevice supportsFamily:MTLGPUFamilyApple7] ? "Yes" : "No");
    }
    
    // Special Rendering Features
    fprintf(gpFile, "Supports Ray Tracing               : %s\n", metalDevice.supportsRaytracing ? "Yes" : "No");
    fprintf(gpFile, "Supports Ray Tracing From Render   : %s\n", metalDevice.supportsRaytracingFromRender ? "Yes" : "No");
    fprintf(gpFile, "------------------------------------------------------------------------------------------------\n");
}

// Overriden implementation of MTKViewDelegate's methods
-(void) mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
    // Code
    CGFloat width = size.width;
    CGFloat height = size.height;

    if (height <= 0.0)
        height = 1.0;

    //* Initialize Metal Viewport
    metalViewport.originX = 0.0;
    metalViewport.originY = 0.0;
    metalViewport.width = width;
    metalViewport.height = height;
    metalViewport.znear = 0.0;
    metalViewport.zfar = 0.0;
}

-(void) drawInMTKView:(nonnull MTKView *)view 
{
    // Code

    //* Get the drawable
    id<CAMetalDrawable> drawable = [self currentDrawable];

    //* Get the current RenderPassDescriptor provided by MTKView
    MTLRenderPassDescriptor *metalRenderPassDescriptor = [self currentRenderPassDescriptor];
    if (drawable == nil || metalRenderPassDescriptor == nil)
        return;

    //* Set RenderPassDescriptor's Color Attachment as :
        //* 1) Color
        //* 2) Texture
    metalRenderPassDescriptor.colorAttachments[0].clearColor = metalClearColor;
    metalRenderPassDescriptor.colorAttachments[0].texture = [drawable texture];

    //* Create a Command Buffer from the command queue
    id<MTLCommandBuffer> metalCommandBuffer = [metalCommandQueue commandBuffer];
    if (metalCommandBuffer == nil)
        return;

    //* Create Render Command Encoder to record drawing commands
    id<MTLRenderCommandEncoder> metalRenderCommandEncoder = [metalCommandBuffer renderCommandEncoderWithDescriptor:metalRenderPassDescriptor];
    if (metalRenderCommandEncoder)
    {
        // Set Viewport of the render command encoder
        [metalRenderCommandEncoder setViewport:metalViewport];

        // Draw Calls

        [metalRenderCommandEncoder endEncoding];
    }

    //* Tell the command buffer to present the current drawable
    [metalCommandBuffer presentDrawable:drawable];

    //* Commit Execution
    [metalCommandBuffer commit];
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
            [NSApp terminate:self];
        break;

        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];
        break;

        default:
        break;
    }
}


-(void) dealloc
{
    // Code
    if (metalCommandQueue)
    {
        [metalCommandQueue release];
        metalCommandQueue = nil;
    }

    if (metalDevice)
    {
        [metalDevice release];
        metalDevice = nil;
    }

    [super dealloc];
}

@end
