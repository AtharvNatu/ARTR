#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Vk.h"

//! Vulkan Related Header Files
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//! Vulkan Related Libraries
#pragma comment(lib, "vulkan-1.lib")

#define WIN_WIDTH   800
#define WIN_HEIGHT  600

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable Declarations
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;
FILE *gpFile = NULL;
WINDOWPLACEMENT wpPrev;
DWORD dwStyle;
const char *gpSzAppName = "ARTR";

//! Vulkan Related Global Variables

//? Instance Extension Related Variables
uint32_t enabledInstanceExtensionCount = 0;
const char *enabledInstanceExtensionNames_array[2]; //* -> VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME

//? Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function Declarations
    VkResult initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);

    // Variable Declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[255];
    BOOL bDone = FALSE;
    VkResult vkResult = VK_SUCCESS;

    // Code
    gpFile = fopen("Log.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Failed To Create Log File ... Exiting !!!"), TEXT("File I/O Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
        fprintf(gpFile, "%s() => Program Started Successfully\n", __func__);

    wsprintf(szAppName, TEXT("%s"), gpSzAppName);

    // Initialization of WNDCLASSEX Structure
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbWndExtra = 0;
    wndclass.cbClsExtra = 0;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ADN_ICON));
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ADN_ICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;

    // Register the class
    RegisterClassEx(&wndclass);

    // Get Screen Co-ordinates
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    // Create Window
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        szAppName,
        TEXT("Atharv Natu : Vulkan Instance"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (screenX / 2) - (WIN_WIDTH / 2),
        (screenY / 2) - (WIN_HEIGHT / 2),
        WIN_WIDTH,
        WIN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ghwnd = hwnd;

    //* Initialize
    vkResult = initialize();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => initialize() Failed !!!\n", __func__);
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
        fprintf(gpFile, "%s() => initialize() Succeeded\n", __func__);

    // Show and Update Window
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    // Bring the window to foreground and set focus
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //* Game Loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                bDone = TRUE;
            
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActiveWindow)
            {
                //* Render the scene
                display();

                //* Update the scene
                update();
            }
        }
    }

    uninitialize();

    return (int)msg.wParam;

}

// Callback Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Function Declarations
    void ToggleFullScreen(void);
    void resize(int, int);
    void uninitialize(void);

    // Code
    switch(iMsg)
    {
        case WM_CREATE:
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

        case WM_SETFOCUS:
            gbActiveWindow = TRUE;
        break;

        case WM_KILLFOCUS:
            gbActiveWindow = FALSE;
        break;

        case WM_SIZE:
            resize(LOWORD(wParam), HIWORD(wParam));
        break;

        case WM_KEYDOWN:

            switch(wParam)
            {
                case 27:
                    DestroyWindow(hwnd);
                break;

                default:
                break;
            }

        break;

        case WM_CHAR:

            switch(wParam)
            {
                case 'F':
                case 'f':
                    ToggleFullScreen();
                break;

                default:
                break;
            }

        break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
        break;

        default:
        break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
    // Variable Declarations
    MONITORINFO mi;

    // Code
    if (gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(
                    ghwnd, 
                    HWND_TOP, 
                    mi.rcMonitor.left, 
                    mi.rcMonitor.top, 
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED
                );
            }

            ShowCursor(FALSE);
            gbFullScreen = TRUE;
        }
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(
            ghwnd, 
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOZORDER
        );

        ShowCursor(TRUE);
        gbFullScreen = FALSE;
    }
}

VkResult initialize(void)
{
    // Function Declarations
    VkResult createVulkanInstance(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => createVulkanInstance() Failed !!!\n", __func__);
    else
        fprintf(gpFile, "%s() => createVulkanInstance() Succeeded\n", __func__);

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
    // Function Declarations
    void ToggleFullScreen(void);

    // Code
    if (gbFullScreen)
        ToggleFullScreen();

    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    //* Step-5 of Instance Creation
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "%s() => vkDestroyInstance() Succeeded\n", __func__);
    }

    if (gpFile)
    {
        fprintf(gpFile, "%s() => Program Terminated Successfully\n", __func__);
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
        fprintf(gpFile, "%s() => fillInstanceExtensionNames() Failed !!!\n", __func__);
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
    VkBool32 win32SurfaceExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
           
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
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

    if (win32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s() => VK_KHR_WIN32_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        fprintf(gpFile, "%s() => VK_KHR_WIN32_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    //* Step - 8
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
        fprintf(gpFile, "%s() => Enabled Vulkan Instance Extension Name : %s\n", __func__, enabledInstanceExtensionNames_array[i]);

    return vkResult;
}


