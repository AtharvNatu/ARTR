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

//? Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//? Vulkan Physical Device Related
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

//? Device Extensions Related Variables
uint32_t enabledDeviceExtensionCount = 0;
const char *enabledDeviceExtensionNames_array[1]; //* -> VK_KHR_SWAPCHAIN_EXTENSTION_NAME

//? Vulkan Device Creation Related Variables
VkDevice vkDevice = VK_NULL_HANDLE;

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
        TEXT("Atharv Natu : Vulkan Logical Device"),
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
        fprintf(gpFile, "%s() => initialize() Failed : %d !!!\n", __func__, vkResult);
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
            memset((void*)&wpPrev, 0, sizeof(WINDOWPLACEMENT));
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
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
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

    //! Enumerate and Selected Required Physical Device and its Queue Family Index
    vkResult = getPhysicalDevice();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => getPhysicalDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => getPhysicalDevice() Succeeded\n", __func__);

    //! Print Vulkan Info
    vkResult = printVkInfo();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => printVkInfo() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => printVkInfo() Succeeded\n", __func__);

    //! Create Vulkan Device
    vkResult = createVulkanDevice();
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => createVulkanDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => createVulkanDevice() Succeeded\n", __func__);

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

    //* Step - 5 of Device Creation (Destroy Vulkan Device)
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpFile, "%s() => vkDeviceWaitIdle() Succeeded\n", __func__);
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpFile, "%s() => vkDestroyDevice() Succeeded\n", __func__);
    }

    //* No need to destroy selected physical device

    //* Step - 5 of Presentation Surface
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
        fprintf(gpFile, "%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
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
        fprintf(gpFile, "%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
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

        fprintf(gpFile, "%s() => Vulkan Instance Extension Name : %s\n", __func__, instanceExtensionNames_array[i]);
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

VkResult getSupportedSurface(void)
{
    // Code

    //* Step - 1
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    VkResult vkResult = VK_SUCCESS;

    //* Step - 2
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    //* Step - 3
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    //* Step - 4
    vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => vkCreateWin32SurfaceKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => vkCreateWin32SurfaceKHR() Succeeded\n", __func__);

    return vkResult;
}

VkResult getPhysicalDevice(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    
    // Code

    //* Step - 2
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (vkResult == VK_SUCCESS)
        fprintf(gpFile, "%s() Call 1 => vkEnumeratePhysicalDevices() Succeeded\n", __func__); 
    else if (physicalDeviceCount == 0)
    {
        fprintf(gpFile, "%s() => vkEnumeratePhysicalDevices() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "%s() Call 1 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //* Step - 3
    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (vkPhysicalDevice_array == NULL)
    {
        fprintf(gpFile, "%s() => malloc() Failed For vkPhysicalDevice_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 4
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (vkResult != VK_SUCCESS)
    {   
        fprintf(gpFile, "%s() Call 2 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
       fprintf(gpFile, "%s() Call 2 => vkEnumeratePhysicalDevices() Succeeded\n", __func__);

    //* Step - 5
    VkBool32 bFound = VK_FALSE;
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        //* Step - 5.1
        uint32_t queueCount = UINT32_MAX;

        //* Step - 5.2
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);

        //* Step - 5.3
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(queueCount * sizeof(VkQueueFamilyProperties));
        if (vkQueueFamilyProperties_array == NULL)
        {
            fprintf(gpFile, "%s() => malloc() Failed For vkQueueFamilyProperties_array !!!\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        //* Step - 5.4
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        //* Step - 5.5
        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
        if (isQueueSurfaceSupported_array == NULL)
        {
            fprintf(gpFile, "%s() => malloc() Failed For isQueueSurfaceSupported_array\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        //* Step - 5.6
        for (uint32_t j = 0; j < queueCount; j++)
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);

        //* Step - 5.7
        for (uint32_t j = 0; j < queueCount; j++)
        {
            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    vkPhysicalDevice_selected = vkPhysicalDevice_array[i];
                    graphicsQueueFamilyIndex_selected = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }

        //* Step - 5.8
        if (isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            fprintf(gpFile, "%s() => free() Succeeded For isQueueSurfaceSupported_array\n", __func__);
            isQueueSurfaceSupported_array = NULL;
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            fprintf(gpFile, "%s() => free() Succeeded For vkQueueFamilyProperties_array\n", __func__);
            vkQueueFamilyProperties_array = NULL;
        }

        //* Step - 5.9
        if (bFound == VK_TRUE)
            break;
        
    }

    //* Step - 5.10
    if (bFound == VK_TRUE)
        fprintf(gpFile, "%s() => Succeeded To Obtain Graphics Supported Physical Device\n", __func__);

    //* Step - 6
    else
    {
        fprintf(gpFile, "%s() => Failed To Obtain Graphics Supported Physical Device !!!\n", __func__);
        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            fprintf(gpFile, "%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
            vkPhysicalDevice_array = NULL;
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    //* Step - 7
    memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    //* Step - 8
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

    //* Step - 9
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures_array;
    memset((void*)&vkPhysicalDeviceFeatures_array, 0, sizeof(VkPhysicalDeviceFeatures));
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures_array);

    //* Step - 10
    if (vkPhysicalDeviceFeatures_array.tessellationShader == VK_TRUE)
        fprintf(gpFile, "%s() => Selected Physical Device Supports Tessellation Shader\n", __func__);
    else
        fprintf(gpFile, "%s() => Selected Physical Device Does Not Support Tessellation Shader !!!\n", __func__);

    if (vkPhysicalDeviceFeatures_array.geometryShader == VK_TRUE)
        fprintf(gpFile, "%s() => Selected Physical Device Supports Geometry Shader\n", __func__);
    else
        fprintf(gpFile, "%s() => Selected Physical Device Does Not Support Geometry Shader !!!\n", __func__);

    return vkResult;
}

VkResult printVkInfo(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpFile, "\nVULKAN INFORMATION\n");
    fprintf(gpFile, "------------------------------------------------------------------------------------------------\n");
    
    //* Step - 3.1
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        //* Step - 3.2
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

        //* Step - 3.3
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
        fprintf(gpFile, "Vulkan API Version : %u.%u.%u\n", majorVersion, minorVersion, patchVersion);

        //* Step - 3.4
        fprintf(gpFile, "Device Name : %s\n", vkPhysicalDeviceProperties.deviceName);

        //* Step - 3.5
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                fprintf(gpFile, "Device Type : Integrated GPU (iGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                fprintf(gpFile, "Device Type : Discrete GPU (dGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                fprintf(gpFile, "Device Type : Virtual GPU (vGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                fprintf(gpFile, "Device Type : CPU\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                fprintf(gpFile, "Device Type : Other\n");
            break;

            default:
                fprintf(gpFile, "Device Type : UNKNOWN\n");
            break;
        }

        //* Step - 3.6
        fprintf(gpFile, "Vendor ID : 0x%4x\n", vkPhysicalDeviceProperties.vendorID);

        //* Step - 3.7
        fprintf(gpFile, "Device ID : 0x%4x\n", vkPhysicalDeviceProperties.deviceID);
    }

    fprintf(gpFile, "------------------------------------------------------------------------------------------------\n\n");

    //* Step - 3.8
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        fprintf(gpFile, "%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
        vkPhysicalDevice_array = NULL;
    }

    return vkResult;
}

VkResult fillDeviceExtensionNames(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t deviceExtensionCount = 0;
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        fprintf(gpFile, "%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        fprintf(gpFile, "%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        fprintf(gpFile, "%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **deviceExtensionNames_array = NULL;
    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    if (deviceExtensionNames_array == NULL)
    {
        fprintf(gpFile, "%s() => malloc() Failed For deviceExtensionNames_array !!!\n", __func__);
        if (vkExtensionProperties_array)
        {
            free(vkExtensionProperties_array);
            vkExtensionProperties_array = NULL;
        }
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
        if (deviceExtensionNames_array[i] == NULL)
        {
            fprintf(gpFile, "%s() => malloc() Failed For deviceExtensionNames_array[%d] !!!\n", __func__, i);
            if (deviceExtensionNames_array)
            {
                free(deviceExtensionNames_array);
                deviceExtensionNames_array = NULL;
            }
            if (vkExtensionProperties_array)
            {
                free(vkExtensionProperties_array);
                vkExtensionProperties_array = NULL;
            }
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

        fprintf(gpFile, "%s() => Vulkan Device Extension Name : %s\n", __func__, deviceExtensionNames_array[i]);
    }

    fprintf(gpFile, "\n------------------------------------------------------------------------------------------------\n");
    fprintf(gpFile, "%s() => Vulkan Device Extension Count : %d\n", __func__, deviceExtensionCount);
    fprintf(gpFile, "------------------------------------------------------------------------------------------------\n\n");

    //* Step - 4
    if (vkExtensionProperties_array)
    {
        free(vkExtensionProperties_array);
        vkExtensionProperties_array = NULL;
    }

    //* Step - 5
    VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;
    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    //* Step - 6
    if (deviceExtensionNames_array)
    {
        for (uint32_t i = 0; i < deviceExtensionCount; i++)
        {
            free(deviceExtensionNames_array[i]);
            deviceExtensionNames_array[i] = NULL;
        }
        free(deviceExtensionNames_array);
        deviceExtensionNames_array = NULL;
    }
    
    //* Step - 7
    if (vulkanSwapchainExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        fprintf(gpFile, "%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Found\n", __func__);

    //* Step - 8
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
        fprintf(gpFile, "%s() => Enabled Vulkan Device Extension Name : %s\n", __func__, enabledDeviceExtensionNames_array[i]);

    return vkResult;

}

VkResult createVulkanDevice(void)
{
    // Function Declaration
    VkResult fillDeviceExtensionNames(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = fillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => fillDeviceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        fprintf(gpFile, "%s() => fillDeviceExtensionNames() Succeeded\n", __func__);

    
    //* Step - 2
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    //? Deprecated in Vulkan Spec
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;

    //* Step - 3
    vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => vkCreateDevice() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        fprintf(gpFile, "%s() => vkCreateDevice() Succeeded\n", __func__);

    return vkResult;
}
