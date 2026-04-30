// Standard Headers
#include <stdio.h>          // For Standard I/O
#include <stdlib.h>         // For exit()
#include <memory.h>         // For memset()

// X11 Headers
#include <X11/Xlib.h>       // For XClient APIs
#include <X11/Xutil.h>      // For XVisualInfo
#include <X11/XKBlib.h>     // For Keyboard
#include <X11/keysym.h>     
#include <X11/Xatom.h>      // For XA_Atom

// Macros
#define WIN_WIDTH       800
#define WIN_HEIGHT      600

// Global Variable Declarations
const char *gpSzAppName = "ARTR";
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap colormap;
Window window;

int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

Bool bActiveWindow = False;
Bool bEscapeKeyPressed = False;
Bool bFullscreen = False;
Bool bWindowMinimized = False;

FILE* gpFile = NULL;

// Entry Point Function
int main(void)
{
    // Function Declarations
    int initialize(void);
    void resize(int, int);
    void display(void);
    void update(void);
    void uninitialize(void);
    void toggleFullScreen(void);
    Bool isWindowMinimized(void);

    // Variable Declarations
    XVisualInfo xVisualInfo;
    int iNumFBConfigs = 0;
    XSetWindowAttributes windowAttributes;
    int defaultScreen;
    int defaultDepth;
    int styleMask;
    Atom windowManagerDeleteAtom;
    int screenWidth, screenHeight;
    XEvent event;
    KeySym keySym;
    char keys[26];
    Bool bDone = False;

    // Code

    //* Log File Creation
    gpFile = fopen("Log.txt", "w");
    if (gpFile == NULL)
    {
        printf("\nFailed To Create Log File ... Exiting Now !!!\n");
        exit(EXIT_FAILURE);
    }
    else
        fprintf(gpFile, "Program Started Successfully => main()\n");

    //* Open the display
    gpDisplay = XOpenDisplay(NULL);
    if (gpDisplay == NULL)
    {
        fprintf(gpFile, "ERROR : XOpenDisplay() Failed !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    //* Get Default Screen From Display
    defaultScreen = XDefaultScreen(gpDisplay);

    //* Get Default Depth From Display and Default Screen


    //* Initialize Local XVisualInfo
    memset((void*)&xVisualInfo, 0, sizeof(XVisualInfo));
    xVisualInfo.screen = defaultScreen;

    gpXVisualInfo = XGetVisualInfo(gpDisplay, VisualScreenMask, &xVisualInfo, &iNumFBConfigs);
    if (gpXVisualInfo == NULL)
    {
        fprintf(gpFile, "ERROR : XGetVisualInfo() Failed !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    fprintf(gpFile, "No. of FB Configs = %d\n", iNumFBConfigs);

    //* Create Color Map
    colormap = XCreateColormap(gpDisplay, 
        XRootWindow(gpDisplay, xVisualInfo.screen), 
        gpXVisualInfo->visual, 
        AllocNone
    );

    //* Initialize Window Attributes
    memset((void*)&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(gpDisplay, defaultScreen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | FocusChangeMask | VisibilityChangeMask | PropertyChangeMask;

    //* Initialize Window Styles in styleMask
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    //* Create the window
    window = XCreateWindow(
        gpDisplay,
        RootWindow(gpDisplay, xVisualInfo.screen),
        0,
        0,
        WIN_WIDTH,
        WIN_HEIGHT,
        0,
        gpXVisualInfo->depth,
        InputOutput,
        gpXVisualInfo->visual,
        styleMask,
        &windowAttributes
    );
    if (!window)
    {
        fprintf(gpFile, "ERROR : XCreateWindow() Failed ... Exiting !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    //* Set Window Caption
    XStoreName(gpDisplay, window, "Atharv Natu : Vulkan");

    //* Prepare Window to respond to Window Manager's Close Event
    windowManagerDeleteAtom = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, window, &windowManagerDeleteAtom, 1);

    //* Show the window
    XMapWindow(gpDisplay, window);

    //* Centering of window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(gpDisplay, defaultScreen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(gpDisplay, defaultScreen));
    XMoveWindow(gpDisplay, window, ((screenWidth / 2) - (WIN_WIDTH / 2)), ((screenHeight / 2) - (WIN_HEIGHT / 2)));

    //! Initialize
    int iResultVal = initialize();
    if (iResultVal != 0)
    {
        fprintf(gpFile, "ERROR : initialize() Failed ... Exiting !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    else
        fprintf(gpFile, "initialize() Succeeded\n");

    //! Event Loop
    while (bDone == False)
    {
        while (XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);

            switch(event.type)
            {
                case MapNotify:
                break;

                case FocusIn:
                    bActiveWindow = True;
                break;

                case FocusOut:
                    bActiveWindow = False;
                break;

                case ConfigureNotify:
                    if (event.xconfigure.width != winWidth || event.xconfigure.height != winHeight)
                    {
                        winWidth = event.xconfigure.width;
                        winHeight = event.xconfigure.height;
                        resize(winWidth, winHeight);
                    }
                break;

                case KeyPress:
                {
                    keySym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keySym)
                    {
                        case XK_Escape:
                            bEscapeKeyPressed = True;
                        break;
                    }

                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                    switch(keys[0])
                    {
                        case 'F':
                        case 'f':
                            if (bFullscreen == False)
                            {
                                toggleFullScreen();
                                bFullscreen = True;
                            }
                            else
                            {
                                toggleFullScreen();
                                bFullscreen = False;
                            }
                        break;
                    }
                }
                break;

                case PropertyNotify:
                    if (isWindowMinimized())
                        bWindowMinimized = True;
                    else
                        bWindowMinimized = False;
                break;

                case DestroyNotify:
                break;

                case 33:
                    bDone = True;
                break;

                default:
                break;
            }
        }

        if (bActiveWindow == True)
        {
            if (bEscapeKeyPressed == True)
            {
                bDone = True;
            }

            if (bWindowMinimized == False)
            {
                //! Render the scene
                display();

                //! Update the scene
                update();
            }
        }
    }

    //! Uninitialize
    uninitialize();

    return 0;
}

void toggleFullScreen(void)
{
    // Variable Declarations
    Atom wm_current_state_atom;
    Atom wm_fullscreen_state_atom;
    XEvent event;

    // Code
    wm_current_state_atom = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    wm_fullscreen_state_atom = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);

    memset(&event, 0, sizeof(XEvent));

    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = wm_current_state_atom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = bFullscreen ? 0 : 1;
    event.xclient.data.l[1] = wm_fullscreen_state_atom;

    XSendEvent(
        gpDisplay, 
        RootWindow(gpDisplay, gpXVisualInfo->screen),
        False,
        SubstructureNotifyMask,
        &event
    );
}

Bool isWindowMinimized(void)
{
    // Function Declarations
    void uninitialize();

    // Variable Declarations
    Bool windowMinimized = False;
    int iResult = 0;
    Atom returned_property_type = None;
    int returned_property_format = -1;
    unsigned long number_of_returned_items = 0;
    unsigned long number_of_bytes_left = 0;
    Atom* returned_property_data_array = NULL;
    
    // Code
    Atom wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", True);
    if (wm_state == None)
    {
        fprintf(gpFile, "ERROR : XInternAtom() Failed For _NET_WM_STATE ... Exiting !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    Atom wm_state_hidden = XInternAtom(gpDisplay, "_NET_WM_STATE_HIDDEN", True);
    if (wm_state_hidden == None)
    {
        fprintf(gpFile, "ERROR : XInternAtom() Failed For _NET_WM_STATE_HIDDEN ... Exiting !!!\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    iResult = XGetWindowProperty(
        gpDisplay,
        window,
        wm_state,
        0L,
        1024,
        False,
        XA_ATOM,
        &returned_property_type,
        &returned_property_format,
        &number_of_returned_items,
        &number_of_bytes_left,
        (unsigned char**)&returned_property_data_array
    );

    if (iResult != 0 || returned_property_data_array == NULL)
    {
        if (returned_property_data_array)
        {
            XFree(returned_property_data_array);
            returned_property_data_array = NULL;
        }

        return False;
    }
    else
    {
        // Loop over the returned array for required Window Property
        for (unsigned long i = 0; i < number_of_returned_items; i++)
        {
            // Check whether the returned array contains "Hidden" Property or not
            if (returned_property_data_array[i] == wm_state_hidden)
            {
                windowMinimized = True;
                break;
            }
        }
    }

    if (returned_property_data_array)
    {
        XFree(returned_property_data_array);
        returned_property_data_array = NULL;
    }

    return windowMinimized;

}

int initialize(void)
{
    // Code
    return 0;
}

void resize(int width, int height)
{
    // Code
    if (height <= 0)
        height = 1;
}

void display(void)
{
    // Code
}

void update(void)
{
    // Code
}

void uninitialize(void)
{
    // Code
    void toggleFullScreen(void);

    // Code
    if (bFullscreen)
    {
        toggleFullScreen();
        bFullscreen = False;
    }

    if (window)
    {
        XDestroyWindow(gpDisplay, window);
    }

    if (colormap)
    {
        XFreeColormap(gpDisplay, colormap);
    }

    if (gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if (gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }

    if (gpFile)
    {
        fprintf(gpFile, "Program Terminated Successfully => uninitialize()\n");
        fclose(gpFile);
        gpFile = NULL;
    }

}
