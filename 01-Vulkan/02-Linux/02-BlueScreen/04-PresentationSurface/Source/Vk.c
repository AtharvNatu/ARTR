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

//! Vulkan Related Header Files
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>

//! GLM Related Macros and Header Files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Macros
#define WIN_WIDTH           1000
#define WIN_HEIGHT          800
#define _ARRAYSIZE(x)       (sizeof(x) / sizeof((x)[0]))

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

//! Vulkan Related Global Variables

//? Instance Extensions Related Variables
uint32_t enabledInstanceExtensionCount = 0;

//* VK_KHR_SURFACE_EXTENSION_NAME
//* VK_KHR_XLIB_SURFACE_EXTENSION_NAME
const char *enabledInstanceExtensionNames_array[2];     

//? Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

//? Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// Entry Point Function
int main(void)
{
    // Function Declarations
    VkResult initialize(void);
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
    VkResult vkResult = VK_SUCCESS;

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
    XStoreName(gpDisplay, window, "Atharv Natu : Vulkan Presentation Surface");

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
    vkResult = initialize();
    if (vkResult != VK_SUCCESS)
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

VkResult initialize(void)
{
    // Function Declarations
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //! Create Vulkan Instance
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => createVulkanInstance() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => createVulkanInstance() Succeeded\n", __func__);

    //! Create Vulkan Presentation Surface
    vkResult = getSupportedSurface();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => getSupportedSurface() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => getSupportedSurface() Succeeded\n", __func__);

    return vkResult;
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

    //* Step-5 of Presentation Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "%s() => vkDestroySurfaceKHR() Succeeded\n", __func__);
    }

    //* Step - 5 of Instance Creation
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "%s() => vkDestroyInstance() Succeeded\n", __func__);
    }

    if (gpFile)
    {
        fprintf(gpFile, "Program Terminated Successfully => uninitialize()\n");
        fclose(gpFile);
        gpFile = NULL;
    }

}

//! Definition of Vulkan Related Functions
VkResult createVulkanInstance(void)
{
    // Function Declarations
    VkResult fillInstanceExtensionNames(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = fillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => fillInstanceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        fprintf(gpFile, "%s() => fillInstanceExtensionNames() Succeeded\n", __func__);

    //* Step - 2
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpSzAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpSzAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

    //* Step - 3
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;
        
    //* Step - 4
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
    if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFile, "%s() => vkCreateInstance() Failed Due To Incompatible Driver : %d!!!\n", __func__, vkResult);
        return vkResult;
    } 
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "%s() => vkCreateInstance() Failed Because Required Extension Is Not Present : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => vkCreateInstance() Failed : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else 
        fprintf(gpFile, "%s() => vkCreateInstance() Succeeded\n", __func__);

    return vkResult;
}

VkResult fillInstanceExtensionNames(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t instanceExtensionCount = 0;
    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Failed !!!\n", __func__);
    else
        fprintf(gpFile, "%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(instanceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        fprintf(gpFile, "%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Failed !!!\n", __func__);
    else
        fprintf(gpFile, "%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **instanceExtensionNames_array = NULL;
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    if (instanceExtensionNames_array == NULL)
    {
        fprintf(gpFile, "%s() => malloc() Failed For instanceExtensionNames_array !!!\n", __func__);
        if (vkExtensionProperties_array)
        {
            free(vkExtensionProperties_array);
            vkExtensionProperties_array = NULL;
        }
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
        if (instanceExtensionNames_array[i] == NULL)
        {
            fprintf(gpFile, "%s() => malloc() Failed For instanceExtensionNames_array[%d] !!!\n", __func__, i);
            if (instanceExtensionNames_array)
            {
                free(instanceExtensionNames_array);
                instanceExtensionNames_array = NULL;
            }
            if (vkExtensionProperties_array)
            {
                free(vkExtensionProperties_array);
                vkExtensionProperties_array = NULL;
            }
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

        fprintf(gpFile, "%s() => Vulkan Extension Name : %s\n", __func__, instanceExtensionNames_array[i]);
    }

    //* Step - 4
    if (vkExtensionProperties_array)
    {
        free(vkExtensionProperties_array);
        vkExtensionProperties_array = NULL;
    }

    //* Step - 5
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 xlibSurfaceExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
           
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0)
        {
            xlibSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        } 
    }

    //* Step - 6
    if (instanceExtensionNames_array)
    {
        for (uint32_t i = 0; i < instanceExtensionCount; i++)
        {
            free(instanceExtensionNames_array[i]);
            instanceExtensionNames_array[i] = NULL;
        }
        free(instanceExtensionNames_array);
        instanceExtensionNames_array = NULL;
    }
    
    //* Step - 7
    if (vulkanSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        fprintf(gpFile, "%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    if (xlibSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s() => VK_KHR_XLIB_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        fprintf(gpFile, "%s() => VK_KHR_XLIB_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    //* Step - 8
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
        fprintf(gpFile, "%s() => Enabled Vulkan Instance Extension Name : %s\n", __func__, enabledInstanceExtensionNames_array[i]);

    return vkResult;
}

VkResult getSupportedSurface(void)
{
    // Code

    //* Step - 1
    VkXlibSurfaceCreateInfoKHR vkXlibSurfaceCreateInfoKHR;
    VkResult vkResult = VK_SUCCESS;

    //* Step - 2
    memset((void*)&vkXlibSurfaceCreateInfoKHR, 0, sizeof(VkXlibSurfaceCreateInfoKHR));

    //* Step - 3
    vkXlibSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    vkXlibSurfaceCreateInfoKHR.pNext = NULL;
    vkXlibSurfaceCreateInfoKHR.flags = 0;
    vkXlibSurfaceCreateInfoKHR.dpy = gpDisplay;
    vkXlibSurfaceCreateInfoKHR.window = window;

    //* Step - 4
    vkResult = vkCreateXlibSurfaceKHR(vkInstance, &vkXlibSurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => vkCreateXlibSurfaceKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => vkCreateXlibSurfaceKHR() Succeeded\n", __func__);

    return vkResult;
}
