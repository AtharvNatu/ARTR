#import "View.h"

// Global Variable Declarations
int winWidth = 0;
int winHeight = 0;

@implementation View
{
    @private
    CADisplayLink* displayLink;
}

-(id) initWithFrame:(CGRect)frameRect
{
    // Variable Declarations
    UITapGestureRecognizer* singleTapGestureRecognizer = nil;
    UITapGestureRecognizer* doubleTapGestureRecognizer = nil;
    UISwipeGestureRecognizer* swipeGestureRecognizer = nil;
    UILongPressGestureRecognizer* longPressGestureRecognizer = nil;
    int result = 0;
    
    // Code
    self = [super initWithFrame:frameRect];
    if (self)
    {
        winWidth = [self bounds].size.width;
        winHeight = [self bounds].size.height;
        
        // Initialize
        result = [self initialize];
        if (result != 0)
            printf("intialize() Failed !!!\n");
        else
            printf("initialize() Succeeded ...\n");
        
        // Become First Responder
        [self becomeFirstResponder];
        
        // Gesture Recognition
        
        // Single Tap
        singleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        // Double Tap
        doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        // Prevent single-tap from triggering during a double-tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        // Swipe
        swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        [swipeGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        // Long Press
        longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        [longPressGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:longPressGestureRecognizer];
        
        // When custom gesture recognizer objects are registered/added to the View (self), it retains them by incrementing the reference count. So, we must decrement it.
        [longPressGestureRecognizer release];
        longPressGestureRecognizer = nil;
        
        [swipeGestureRecognizer release];
        swipeGestureRecognizer = nil;
        
        [doubleTapGestureRecognizer release];
        doubleTapGestureRecognizer = nil;
        
        [singleTapGestureRecognizer release];
        singleTapGestureRecognizer = nil;
        
        // Initialize Display Link
        NSUInteger animationFrameInterval = 60;
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    
    return self;
}

+(Class) layerClass
{
    // Code
    return [CAMetalLayer class];
}

/*
 * Implement this method only when custom drawing is required.
 * Do not leave it as an empty stub. An empty implementation causes both
 * drawRect() and the renderer's render() method to perform painting during
 * animations, resulting in unnecessary concurrent paint operations and
 * degraded performance. If custom drawing is not needed, do not implement
 * this method.
 
-(void) drawRect:(CGRect)rect
{
    // Code
}
 */

-(void) drawView
{
    // Code
    [self render];
    
    [self update];
}

-(void) layoutSubviews
{
    // Code
    [super layoutSubviews];
    
    // Set winWidth and winHeight according to the view's layout
    winWidth = [self bounds].size.width * [self contentScaleFactor];
    winHeight = [self bounds].size.height * [self contentScaleFactor];
    
    CGSize size = CGSizeMake(winWidth, winHeight);
    [((CAMetalLayer*)[self layer])setDrawableSize:size];
    
    // Resize
    [self resize:winWidth :winHeight];
}

-(BOOL) canBecomeFirstResponder
{
    // Code
    return YES;
}

-(void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    // Code
}

-(void) onSingleTap:(UITapGestureRecognizer*)gestureRecognizer
{
    // Code
}

-(void) onDoubleTap:(UITapGestureRecognizer*)gestureRecognizer
{
    // Code
}

-(void) onSwipe:(UISwipeGestureRecognizer*)gestureRecognizer
{
    // Code
    [self uninitialize];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer*)gestureRecognizer
{
    // Code
}

-(void) dealloc
{
    // Code
    [super dealloc];
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

-(void) render
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
    if (displayLink)
    {
        [displayLink release];
        displayLink = nil;
    }
}

@end
