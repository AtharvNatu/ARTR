//* Android Header Files
#include <android_native_app_glue.h>        // Everything related with pure native activity needs this wrapper
#include <android/log.h>                    // For android_log_print()

// C/C++ Header Files
#include <stdlib.h>
#include <memory.h>                         // For memset()
#include <math.h>                           // For sqrtf()

//! Vulkan Related Header Files
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>

//! GLM Related Macros and Header Files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Macros
#define _ARRAYSIZE(x)       (sizeof(x) / sizeof((x)[0]))

//* Android Global Variables
typedef struct 
{
    struct android_app *app;
    bool bActive;
} Engine;

ANativeWindow *androidNativeWindow = NULL;
AAssetManager *androidAssetManager = NULL;
long touchStartTime = 0;
long pendingSingleTapTime = 0;
bool bTouchDown = false;
bool bDragging = false;
bool bLongPressDetected = false;
bool bDoubleTapDetected = false;
bool bPendingSingleTap = false;
float touchStartX = 0.0f, touchStartY = 0.0f;
float lastTapX = 0.0f, lastTapY = 0.0f;

//* Android Callbacks
void engine_handle_cmd(struct android_app*, int32_t);
int32_t engine_handle_input(struct android_app*, AInputEvent*);

//! Vulkan Related Global Variables

//? Instance Extension Related Variables
uint32_t enabledInstanceExtensionCount = 0;

//* VK_KHR_SURFACE_EXTENSION_NAME,
//* VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
//* VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char *enabledInstanceExtensionNames_array[3];

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

//? Vulkan Device Queue Related Variables
VkQueue vkQueue = VK_NULL_HANDLE;

//? Color Format and Color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//? Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//? Swapchain
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

//? Swapchain Images and Image Views
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

//? For Depth Image
VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;

//? Command Pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//? Render Pass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//? Frame Buffer
VkFramebuffer *vkFramebuffer_array = NULL;

//? Fences and Semaphores
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;
VkFence *vkFence_array = NULL;

//? Clear Color Values
VkClearColorValue vkClearColorValue;
VkClearDepthStencilValue vkClearDepthStencilValue;

//? Render
bool bInitialized = false;
uint32_t currentImageIndex = UINT32_MAX;

//? Validation
bool bValidation = true;
uint32_t enabledValidationLayerCount = 0;
const char *enabledValidationLayerNames_array[1];   //* For VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;
//? Vertex Buffer Related Variables
typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

//? Uniform Related Variables
typedef struct
{
    glm::mat4 mvpMatrix;
    glm::vec4 color;
} Host_UniformData;

typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

UniformData uniformData;

//? Shader Related Variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//? DescriptorSetLayout Related Variables
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

//? PipelineLayout Related Variables
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//? Descriptor Pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//? Descriptor Set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

//? Pipeline Related Variables
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

//* Sine Wave Related Variables
float position_64_graphics[64][64][4];
VertexData vertexData_position_64x64_graphics;
VkCommandBuffer* vkCommandBuffer_64x64_graphics_array = NULL;

float position_128_graphics[128][128][4];
VertexData vertexData_position_128x128_graphics;
VkCommandBuffer* vkCommandBuffer_128x128_graphics_array = NULL;

float position_256_graphics[256][256][4];
VertexData vertexData_position_256x256_graphics;
VkCommandBuffer* vkCommandBuffer_256x256_graphics_array = NULL;

float position_512_graphics[512][512][4];
VertexData vertexData_position_512x512_graphics;
VkCommandBuffer* vkCommandBuffer_512x512_graphics_array = NULL;

float position_1024_graphics[2048][2048][4];
VertexData vertexData_position_1024x1024_graphics;
VkCommandBuffer* vkCommandBuffer_1024x1024_graphics_array = NULL;


bool bMesh64Chosen = false;
bool bMesh128Chosen = false;
bool bMesh256Chosen = false;
bool bMesh512Chosen = false;
bool bMesh1024Chosen = true;
bool bMesh2048Chosen = false;
bool bMesh4096Chosen = false;

bool bUseCompute = false;

char selectedColor = 'O';
float fAnimationSpeed = 0.0f;

//! Compute Related
typedef struct
{
    int width;
    int height;
    float time;
} PushConstants;

typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} ComputeData;

VkCommandBuffer vkCommandBuffer_compute = VK_NULL_HANDLE;               // For storing dispatch command and barriers
VkShaderModule vkShaderModule_compute_shader = VK_NULL_HANDLE;
VkDescriptorSetLayout vkDescriptorSetLayout_compute = VK_NULL_HANDLE;   // Compute Shader binding layout
VkDescriptorSet vkDescriptorSet_compute = VK_NULL_HANDLE;               // Compute Shader bindings
VkPipelineLayout vkPipelineLayout_compute = VK_NULL_HANDLE;             // Compute Pipeline layout
VkPipeline vkPipeline_compute = VK_NULL_HANDLE;                         // Compute Pipeline for Sinewave

ComputeData computeBuffer;

bool bDone = false;
const char* gpSzAppName = "ARTR";
int winWidth = 0, winHeight = 0;
int singleTap = 0;

//* Entry-Point Function
void android_main(struct android_app* state)
{
    // Function Declarations
    VkResult display(void);
    void update(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Fullscreen and Hiding Status Bar
    JavaVM *vm = state->activity->vm;
    JNIEnv *env = NULL;

    //* android_main runs on a separate native thread (not the Java UI thread)
    vm->AttachCurrentThread(&env, NULL);

    jobject activityObject = state->activity->clazz;
    jclass activityClass = env->GetObjectClass(activityObject);

    jclass windowClass = env->FindClass("android/view/Window");
    jclass viewClass = env->FindClass("android/view/View");

    jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
    jobject windowObject = env->CallObjectMethod(activityObject, getWindowMethod);

    jmethodID getDecorViewMethod = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorViewObject = env->CallObjectMethod(windowObject, getDecorViewMethod);

    //* Get 8 View Class Static Fields
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_LOW_PROFILE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LOW_PROFILE", "I"));
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I"));

    jmethodID setSystemUiVisibilityMethod = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");
    env->CallVoidMethod(
        decorViewObject, 
        setSystemUiVisibilityMethod, 
        flag_SYSTEM_UI_FLAG_IMMERSIVE |
        flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
        flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_LOW_PROFILE |
        flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY
    );

    //* Notch Fix For Fullscreen
    jclass layoutParamsClass = env->FindClass("android/view/WindowManager$LayoutParams");

    jmethodID getAttributesMethod = env->GetMethodID(windowClass, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
    jobject layoutParams = env->CallObjectMethod(windowObject, getAttributesMethod);

    jfieldID layoutInDisplayCutoutModeField = env->GetFieldID(layoutParamsClass, "layoutInDisplayCutoutMode", "I");
    jfieldID shortEdgesField = env->GetStaticFieldID(layoutParamsClass, "LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES", "I");
    jint shortEdges = env->GetStaticIntField(layoutParamsClass, shortEdgesField);
    env->SetIntField(layoutParams, layoutInDisplayCutoutModeField, shortEdges);

    jmethodID setAttributesMethod = env->GetMethodID(windowClass, "setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V");
    env->CallVoidMethod(windowObject, setAttributesMethod, layoutParams);

    //* Change To Landscape Mode
    jclass activityInfoClass = env->FindClass("android/content/pm/ActivityInfo");
    const int flag_SCREEN_ORIENTATION_LANDSCAPE = env->GetStaticIntField(activityInfoClass, env->GetStaticFieldID(activityInfoClass, "SCREEN_ORIENTATION_LANDSCAPE", "I"));

    jmethodID setRequestedOrientationMethod = env->GetMethodID(activityClass, "setRequestedOrientation", "(I)V");
    env->CallVoidMethod(activityObject, setRequestedOrientationMethod, flag_SCREEN_ORIENTATION_LANDSCAPE);

    //* Detach VM from current thread
    vm->DetachCurrentThread();

    Engine engine;
    memset((void*)&engine, 0, sizeof(Engine));

    // Initialize State
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.app = state;

    while (1)
    {
        int identifier;
        struct android_poll_source* source = NULL;

        while ((identifier = ALooper_pollOnce(engine.bActive ? 0 : -1, NULL, NULL, (void**)&source)) >= 0)
        {
            // Process System Events
            if (source != NULL)
            {
                source->process(state, source);
            }

            // Check when to exit
            if (state->destroyRequested)
                return;
        }

        //* Handle Long Press Event
        if (bTouchDown == true && bDragging == false && bLongPressDetected == false && bDoubleTapDetected == false)
        {
            struct timespec ts;
            memset((void*)&ts, 0, sizeof(timespec));
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
            if (now - touchStartTime > 500)
                bLongPressDetected = true;
        }

        //* Handle Single Tap Event
        if (bPendingSingleTap)
        {
            struct timespec ts;
            memset((void*)&ts, 0, sizeof(timespec));
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
            if (now - pendingSingleTapTime > 300)
            {
                bPendingSingleTap = false;

                singleTap++;
                switch(singleTap)
                {
                    case 1:
                        bMesh64Chosen = true;
                        bMesh128Chosen = false;
                        bMesh256Chosen = false;
                        bMesh512Chosen = false;
                        bMesh1024Chosen = false;
                    break;

                    case 2:
                        bMesh64Chosen = false;
                        bMesh128Chosen = true;
                        bMesh256Chosen = false;
                        bMesh512Chosen = false;
                        bMesh1024Chosen = false;
                    break;

                    case 3:
                        bMesh64Chosen = false;
                        bMesh128Chosen = false;
                        bMesh256Chosen = true;
                        bMesh512Chosen = false;
                        bMesh1024Chosen = false;
                    break;

                    case 4:
                        bMesh64Chosen = false;
                        bMesh128Chosen = false;
                        bMesh256Chosen = false;
                        bMesh512Chosen = true;
                        bMesh1024Chosen = false;
                    break;

                    case 5:
                        bMesh64Chosen = false;
                        bMesh128Chosen = false;
                        bMesh256Chosen = false;
                        bMesh512Chosen = false;
                        bMesh1024Chosen = true;
                    break;

                    default:
                        singleTap = 1;
                    break;
                }
            }
                
        }

        //! Game Loop
        if (engine.bActive && bInitialized)
        {
            // Display
            vkResult = display();
            if (vkResult != VK_FALSE && vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => display() Failed : %d !!!\n", __func__, vkResult);
                bDone = true;
            }

            //! Update the scene
            update();
        }

    }
}

void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    // Function Declarations
    VkResult initialize(void);
    VkResult uninitialize(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    Engine *engine = (Engine*)app->userData;

    switch(cmd)
    {
        case APP_CMD_SAVE_STATE:
            engine->bActive = false;
        break;

        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL)
            {
                androidNativeWindow = engine->app->window;
                androidAssetManager = engine->app->activity->assetManager;

                winWidth = ANativeWindow_getWidth(androidNativeWindow);
                winHeight = ANativeWindow_getHeight(androidNativeWindow);

                if (bInitialized == false)
                {
                    vkResult = initialize();
                    if (vkResult != VK_SUCCESS)
                    {
                        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => initialize() Failed : %d !!!\n", __func__, vkResult);
                    }
                    else
                        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => initialize() Succeeded\n", __func__);
                }

                engine->bActive = true;
            }
            else
                androidNativeWindow = NULL;
        break;

        case APP_CMD_TERM_WINDOW:
            vkResult = uninitialize();
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => uninitialize() Failed : %d !!!\n", __func__, vkResult);
            }
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => uninitialize() Succeeded\n", __func__);
        break;

        case APP_CMD_GAINED_FOCUS:
            engine->bActive = true;
        break;

        case APP_CMD_LOST_FOCUS:
            engine->bActive = false;
        break;
    }

}

int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    // Code
    Engine *engine = (Engine*)app->userData;

    int32_t eventType = AInputEvent_getType(event);
    switch(eventType)
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
            switch(action)
            {
                case AMOTION_EVENT_ACTION_DOWN:
                {
                    touchStartX = AMotionEvent_getX(event, 0);
                    touchStartY = AMotionEvent_getY(event, 0);

                    struct timespec ts;
                    memset((void*)&ts, 0, sizeof(timespec));
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    touchStartTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

                    bTouchDown = true;
                    bDragging = false;
                    bLongPressDetected = false;
                    bDoubleTapDetected = false;

                    //* Check whether the down action is for second tap of the double tap
                    if (bPendingSingleTap)
                    {
                        long timeSinceLastTap = touchStartTime - pendingSingleTapTime;
                        float dx = touchStartX - lastTapX;
                        float dy = touchStartY - lastTapY;
                        float distance = sqrtf(dx * dx + dy * dy);
                        if (timeSinceLastTap < 300 && distance < 100.0f)
                        {
                            //* Double Tap Detected
                            bDoubleTapDetected = true;

                            bUseCompute = !bUseCompute;
                        }  
                        else
                        {
                            //* Not a double tap - maybe fling | long tap | single tap  
                        }
                    }

                    bPendingSingleTap = false;
                    break;
                }
                

                case AMOTION_EVENT_ACTION_MOVE:
                {
                    float currentX = AMotionEvent_getX(event, 0);
                    float currentY = AMotionEvent_getY(event, 0);
                    float dx = currentX - touchStartX;
                    float dy = currentY - touchStartY;
                    float distance = sqrtf(dx * dx + dy * dy);
                    if (distance > 50.0f)
                        bDragging = true;
                    break;
                }
                

                case AMOTION_EVENT_ACTION_UP:
                {
                    struct timespec ts;
                    memset((void*)&ts, 0, sizeof(timespec));
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    long currentTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
                    long duration = currentTime - touchStartTime;
                    float endX = AMotionEvent_getX(event, 0);
                    float endY = AMotionEvent_getY(event, 0);
                    float dx = endX - touchStartX;
                    float dy = endY - touchStartY;
                    float distance = sqrtf(dx * dx + dy * dy);
                    bTouchDown = false;
                    if (bDoubleTapDetected)
                    {
                        //* This up belongs to second tap of double tap
                    }
                    else if (bLongPressDetected)
                    {
                        //* This up belongs to long press (already handled) 
                    }
                    else if (bDragging && distance > 150.0f)
                    {
                        //* Swipe Event
                        ANativeActivity_finish(engine->app->activity);
                    }
                    else if (duration < 300.0f && distance < 50.0f)
                    {
                        //* Short tap is detected and second tap may be pending (this may be up of single tap - already handled in main loop)
                        bPendingSingleTap = true;
                        pendingSingleTapTime = currentTime;
                        lastTapX = endX;
                        lastTapY = endY;
                    }
                    bDragging = false;
                    break;
                }
                
            }
            break;
        }
        
    }

    return 0;
}

VkResult initialize(void)
{
    // Function Declarations
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(VkCommandBuffer**);
    void initializeSinewaveArrays(uint32_t, uint32_t);
    VkResult createVertexBuffer(uint32_t, uint32_t, VertexData*);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFramebuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);

    //! Compute Related
    VkResult initialize_compute(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVulkanInstance() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVulkanInstance() Succeeded\n", __func__);

    //! Create Vulkan Presentation Surface
    vkResult = getSupportedSurface();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getSupportedSurface() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getSupportedSurface() Succeeded\n", __func__);

    //! Enumerate and Select Required Physical Device and its Queue Family Index
    vkResult = getPhysicalDevice();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDevice() Succeeded\n", __func__);

    //! Print Vulkan Info
    vkResult = printVkInfo();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => printVkInfo() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => printVkInfo() Succeeded\n", __func__);

    //! Create Vulkan Device
    vkResult = createVulkanDevice();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVulkanDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVulkanDevice() Succeeded\n", __func__);

    //! Get Device Queue
    getDeviceQueue();

    //! Create Swapchain
    vkResult = createSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createSwapchain() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createSwapchain() Succeeded\n", __func__);

    //! Create Swapchain Image and Image Views
    vkResult = createImagesAndImageViews();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createImagesAndImageViews() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createImagesAndImageViews() Succeeded\n", __func__);
    
    //! Create Command Pool
    vkResult = createCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandPool() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandPool() Succeeded\n", __func__);

    //! Create Command Buffers
    vkResult = createCommandBuffers(&vkCommandBuffer_64x64_graphics_array);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_64x64_graphics_array : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Succeeded For vkCommandBuffer_64x64_graphics_array\n", __func__);
    
    vkResult = createCommandBuffers(&vkCommandBuffer_128x128_graphics_array);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_128x128_graphics_array : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Succeeded For vkCommandBuffer_128x128_graphics_array\n", __func__);
    
    vkResult = createCommandBuffers(&vkCommandBuffer_256x256_graphics_array);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_256x256_graphics_array : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Succeeded For vkCommandBuffer_256x256_graphics_array\n", __func__);
    
    vkResult = createCommandBuffers(&vkCommandBuffer_512x512_graphics_array);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_512x512_graphics_array : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Succeeded For vkCommandBuffer_512x512_graphics_array\n", __func__);
    
    vkResult = createCommandBuffers(&vkCommandBuffer_1024x1024_graphics_array);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_1024x1024_graphics_array : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Succeeded For vkCommandBuffer_1024x1024_graphics_array\n", __func__);

    //! Initialize Sinewave Arrays - 64, 128, 256, 512, 1024
    initializeSinewaveArrays(64, 64);
    initializeSinewaveArrays(128, 128);
    initializeSinewaveArrays(256, 256);
    initializeSinewaveArrays(512, 512);
    initializeSinewaveArrays(1024, 1024);

    //! Create Vertex Buffers
    memset((void*)&vertexData_position_64x64_graphics, 0, sizeof(VertexData));
    vkResult = createVertexBuffer(64, 64, &vertexData_position_64x64_graphics);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Failed For vertexData_position_64x64_graphics : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Succeeded For vertexData_position_64x64_graphics\n", __func__);

    memset((void*)&vertexData_position_128x128_graphics, 0, sizeof(VertexData));
    vkResult = createVertexBuffer(128, 128, &vertexData_position_128x128_graphics);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Failed For vertexData_position_128x128_graphics : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Succeeded For vertexData_position_128x128_graphics\n", __func__);
    
    memset((void*)&vertexData_position_256x256_graphics, 0, sizeof(VertexData));
    vkResult = createVertexBuffer(256, 256, &vertexData_position_256x256_graphics);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Failed For vertexData_position_256x256_graphics : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Succeeded For vertexData_position_256x256_graphics\n", __func__);
    
    memset((void*)&vertexData_position_512x512_graphics, 0, sizeof(VertexData));
    vkResult = createVertexBuffer(512, 512, &vertexData_position_512x512_graphics);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Failed For vertexData_position_512x512_graphics : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Succeeded For vertexData_position_512x512_graphics\n", __func__);
    
    memset((void*)&vertexData_position_1024x1024_graphics, 0, sizeof(VertexData));
    vkResult = createVertexBuffer(2048, 2048, &vertexData_position_1024x1024_graphics);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Failed For vertexData_position_1024x1024_graphics : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createVertexBuffer() Succeeded For vertexData_position_1024x1024_graphics\n", __func__);

    //! Create Uniform Buffer
    vkResult = createUniformBuffer();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createUniformBuffer() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createUniformBuffer() Succeeded\n", __func__);

    //! Create Shaders
    vkResult = createShaders();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createShaders() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createShaders() Succeeded\n", __func__);

    //! Create DescriptorSetLayout
    vkResult = createDescriptorSetLayout();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSetLayout() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSetLayout() Succeeded\n", __func__);

    //! Create Pipeline Layout
    vkResult = createPipelineLayout();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipelineLayout() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipelineLayout() Succeeded\n", __func__);

    //! Create Descriptor Pool
    vkResult = createDescriptorPool();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorPool() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorPool() Succeeded\n", __func__);

    //! Create Descriptor Set
    vkResult = createDescriptorSet();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSet() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSet() Succeeded\n", __func__);

    //! Create Render Pass
    vkResult = createRenderPass();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createRenderPass() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createRenderPass() Succeeded\n", __func__);

    //! Create Pipeline
    vkResult = createPipeline();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipeline() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipeline() Succeeded\n", __func__);

    //! Create Framebuffers
    vkResult = createFramebuffers();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createFramebuffers() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createFramebuffers() Succeeded\n", __func__);

    //! Create Semaphores
    vkResult = createSemaphores();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createSemaphores() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createSemaphores() Succeeded\n", __func__);

    //! Create Fences
    vkResult = createFences();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createFences() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createFences() Succeeded\n", __func__);

    //! Initialize Clear Color Values (Analogous to glClearColor())
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;    //* R
    vkClearColorValue.float32[1] = 0.0f;    //* G
    vkClearColorValue.float32[2] = 0.0f;    //* B
    vkClearColorValue.float32[3] = 1.0f;    //* A

    //! Set Default Clear Depth and Stencil Values
    memset((void*)&vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));
    vkClearDepthStencilValue.depth = 1.0f;
    vkClearDepthStencilValue.stencil = 0;

    vkResult = buildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffers() Failed\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffers() Succeeded\n", __func__);

    vkResult = initialize_compute();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => initialize_compute() Failed\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => initialize_compute() Succeeded\n", __func__);

    //! Initialization Completed
    bInitialized = true;
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Initialization Completed Successfully\n", __func__);
    
    return vkResult;
}

VkResult resize(int width, int height)
{
    // Function Declarations
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandBuffers(VkCommandBuffer**);
    VkResult createPipelineLayout(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFramebuffers(void);
    VkResult buildCommandBuffers(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (height <= 0)
        height = 1;

    //* Check the bInitialized Variable
    if (bInitialized == false)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Initialization Not Yet Completed or Failed !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    //* As recreation of swapchain is needed, we are going to repeat many steps of initialize() again. Hence, set bInitialize = false again
    bInitialized = false;
    {
        //* Set Global winWidth and winHeight variables
        winWidth = width;
        winHeight = height;

        //? DESTROY
        //?--------------------------------------------------------------------------------------------------
        //* Wait for device to complete in-hand tasks
        if (vkDevice)
            vkDeviceWaitIdle(vkDevice);
        
        //* Check presence of swapchain
        if (vkSwapchainKHR == VK_NULL_HANDLE)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Swapchain is already NULL ... cannot proceed !!!\n", __func__);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }

        //* Destroy Framebuffer
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
        if (vkFramebuffer_array)
        {
            free(vkFramebuffer_array);
            vkFramebuffer_array = NULL;
        }

        //* Destroy Command Buffer
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_1024x1024_graphics_array[i]);
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_512x512_graphics_array[i]);
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_256x256_graphics_array[i]);
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_128x128_graphics_array[i]);
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_64x64_graphics_array[i]);
        }
            
        if (vkCommandBuffer_1024x1024_graphics_array)
        {
            free(vkCommandBuffer_1024x1024_graphics_array);
            vkCommandBuffer_1024x1024_graphics_array = NULL;
        }
            
        if (vkCommandBuffer_512x512_graphics_array)
        {
            free(vkCommandBuffer_512x512_graphics_array);
            vkCommandBuffer_512x512_graphics_array = NULL;
        }
            
        if (vkCommandBuffer_256x256_graphics_array)
        {
            free(vkCommandBuffer_256x256_graphics_array);
            vkCommandBuffer_256x256_graphics_array = NULL;
        }
            
        if (vkCommandBuffer_128x128_graphics_array)
        {
            free(vkCommandBuffer_128x128_graphics_array);
            vkCommandBuffer_128x128_graphics_array = NULL;
        }
            
        if (vkCommandBuffer_64x64_graphics_array)
        {
            free(vkCommandBuffer_64x64_graphics_array);
            vkCommandBuffer_64x64_graphics_array = NULL;
        }

        //* Destroy PipelineLayout
        if (vkPipelineLayout)
        {
            vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
            vkPipelineLayout = VK_NULL_HANDLE;
        }

        //* Destroy Pipeline
        if (vkPipeline)
        {
            vkDestroyPipeline(vkDevice, vkPipeline, NULL);
            vkPipeline = VK_NULL_HANDLE;
        }

        //* Destroy Render Pass
        if (vkRenderPass)
        {
            vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
            vkRenderPass = VK_NULL_HANDLE;
        }

        //* Destroying Depth Image
        if (vkImageView_depth)
        {
            vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
            vkImageView_depth = VK_NULL_HANDLE;
        }

        if (vkImage_depth)
        {
            vkDestroyImage(vkDevice, vkImage_depth, NULL);
            vkImage_depth = VK_NULL_HANDLE;
        }

        if (vkDeviceMemory_depth)
        {
            vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
            vkDeviceMemory_depth = VK_NULL_HANDLE;
        }

        
        //* Destroy Swapchain Image and Image Views
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);

        if (swapchainImageView_array)
        {
            free(swapchainImageView_array);
            swapchainImageView_array = NULL;
        }

        //! No need to free swapchain images -> Uncommenting causes the code to crash
        // for (uint32_t i = 0; i < swapchainImageCount; i++)
        // {
        //     vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
        //     __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyImage() Succeeded\n", __func__);
        // } 

        if (swapchainImage_array)
        {
            free(swapchainImage_array);
            swapchainImage_array = NULL;
        }

        //* Destroy Swapchain
        if (vkSwapchainKHR)
        {
            vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
            vkSwapchainKHR = VK_NULL_HANDLE;
        }
        //?--------------------------------------------------------------------------------------------------
        
        //? RECREATE FOR RESIZE
        //?--------------------------------------------------------------------------------------------------
        //* Create Swapchain
        vkResult = createSwapchain(VK_FALSE);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createSwapchain() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Swapchain Image and Image Views
        vkResult = createImagesAndImageViews();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createImagesAndImageViews() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Render Pass
        vkResult = createRenderPass();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createRenderPass() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Pipeline Layout
        vkResult = createPipelineLayout();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipelineLayout() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Pipeline
        vkResult = createPipeline();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipeline() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Command Buffers
        vkResult = createCommandBuffers(&vkCommandBuffer_64x64_graphics_array);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_64x64_graphics_array : %d !!!\n", __func__, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        
        vkResult = createCommandBuffers(&vkCommandBuffer_128x128_graphics_array);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_128x128_graphics_array : %d !!!\n", __func__, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        
        vkResult = createCommandBuffers(&vkCommandBuffer_256x256_graphics_array);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_256x256_graphics_array : %d !!!\n", __func__, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        
        vkResult = createCommandBuffers(&vkCommandBuffer_512x512_graphics_array);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_512x512_graphics_array : %d !!!\n", __func__, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        
        vkResult = createCommandBuffers(&vkCommandBuffer_1024x1024_graphics_array);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffers() Failed For vkCommandBuffer_1024x1024_graphics_array : %d !!!\n", __func__, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }

        //* Create Framebuffers
        vkResult = createFramebuffers();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createFramebuffers() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Build Command Buffers
        vkResult = buildCommandBuffers();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffers() Failed\n", __func__);
            return vkResult;
        }
        //?--------------------------------------------------------------------------------------------------
    }
    bInitialized = true;

    return vkResult;
}

VkResult display(void)
{
    // Function Declarations
    VkResult resize(int, int);
    VkResult updateUniformBuffer(void);
    VkResult buildCommandBuffers(void);
    VkResult buildCommandBuffer_compute(void);

    // Variable Declarations
    VkCommandBuffer* vkCommandBuffer_array = NULL;
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (bInitialized == false)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Initialization Not Yet Completed !!!\n", __func__);
        return (VkResult)VK_FALSE;
    }

     if (bMesh64Chosen)
        vkCommandBuffer_array = vkCommandBuffer_64x64_graphics_array;
    else if (bMesh128Chosen)
        vkCommandBuffer_array = vkCommandBuffer_128x128_graphics_array;
    else if (bMesh256Chosen)
        vkCommandBuffer_array = vkCommandBuffer_256x256_graphics_array;
    else if (bMesh512Chosen)
        vkCommandBuffer_array = vkCommandBuffer_512x512_graphics_array;
    else if (bMesh1024Chosen)
        vkCommandBuffer_array = vkCommandBuffer_1024x1024_graphics_array;

    vkResult = buildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffers() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    if (bUseCompute)
    {
        vkResult = buildCommandBuffer_compute();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffer_compute() Failed : %d\n", __func__, vkResult);
            return vkResult;
        }
    }

    //! Acquire next image index
    vkResult = vkAcquireNextImageKHR(vkDevice, vkSwapchainKHR, UINT64_MAX, vkSemaphore_backBuffer, VK_NULL_HANDLE, &currentImageIndex);
    if (vkResult != VK_SUCCESS && vkResult != VK_SUBOPTIMAL_KHR)
    {
        if (vkResult == VK_ERROR_OUT_OF_DATE_KHR)
            resize(winWidth, winHeight);
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAcquireNextImageKHR() Failed : %d\n", __func__, vkResult);
            return vkResult;
        }
    }

    //! Use fence to allow host to wait for completion of execution of previous command buffer
    vkResult = vkWaitForFences(vkDevice, 1, &vkFence_array[currentImageIndex], VK_TRUE, UINT64_MAX);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkWaitForFences() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    //! Make sure fences are ready for execution of next command buffer
    vkResult = vkResetFences(vkDevice, 1, &vkFence_array[currentImageIndex]);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkResetFences() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    //! Render color attachment
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    //! Declare and initialize VkSubmitInfo stucture
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backBuffer;
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;

    if (bUseCompute)
    {
        VkCommandBuffer vkCommandBufferArr[2] =
        {
            vkCommandBuffer_compute,
            vkCommandBuffer_array[currentImageIndex]
        };

        vkSubmitInfo.commandBufferCount = 2;
        vkSubmitInfo.pCommandBuffers = vkCommandBufferArr;
    }
    else
    {
        vkSubmitInfo.commandBufferCount = 1;
        vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
    }

    //! Submit above work to the queue
    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_array[currentImageIndex]);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkQueueSubmit() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    //! Present Rendered Image
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;

    //! Present the queue
    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
    if (vkResult != VK_SUCCESS && vkResult != VK_SUBOPTIMAL_KHR)
    {
        if (vkResult == VK_ERROR_OUT_OF_DATE_KHR)
            resize(winWidth, winHeight);
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkQueuePresentKHR() Failed : %d\n", __func__, vkResult);
            return vkResult;
        }
    }

    vkResult = updateUniformBuffer();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => updateUniformBuffer() Failed : %d\n", __func__, vkResult);

    vkDeviceWaitIdle(vkDevice);

    return vkResult;
}

void update(void)
{
    // Code
    fAnimationSpeed = fAnimationSpeed + 0.02f;
}

VkResult uninitialize(void)
{
    // Function Declarations
    void uninitialize_compute(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 5 of Device Creation (Destroy Vulkan Device)
    //! vkDeviceWaitIdle(vkDevice) should be the 1st API to maintain synchronization
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDeviceWaitIdle() Succeeded\n", __func__);
    }

    uninitialize_compute();

    //* Step - 7 of Fences and Semaphores
    if (vkFence_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFence(vkDevice, vkFence_array[i], NULL);
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyFence() Succeeded For Index : %d\n", __func__, i);
        }

        free(vkFence_array);
        vkFence_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkFence_array\n", __func__);
    }

    if (vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroySemaphore() Succeeded For vkSemaphore_renderComplete\n", __func__);
        vkSemaphore_renderComplete = VK_NULL_HANDLE;
    }

    if (vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroySemaphore() Succeeded For vkSemaphore_backBuffer\n", __func__);
        vkSemaphore_backBuffer = VK_NULL_HANDLE;
    }

    //* Step - 5 of Frame Buffer
    if (vkFramebuffer_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyFramebuffer() Succeeded For Index : %d\n", __func__, i);
        }

        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkFramebuffer_array\n", __func__);
    }

    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyPipeline() Succeeded\n", __func__);
    }

    //* Step - 6 of Render Pass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyRenderPass() Succeeded\n", __func__);
    }

    //* Destroy Descriptor Pool (Destroys Descriptor Set with it)
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        vkDescriptorSet = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyDescriptorPool() => Destroyed vkDescriptorPool and vkDescriptorSet Successfully\n", __func__);
    }

    //* Step - 5 of PipelineLayout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyPipelineLayout() Succeeded\n", __func__);
    }

    //* Step - 5 of DescriptorSetLayout
    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyDescriptorSetLayout() Succeeded\n", __func__);
    }

    //* Step - 11 of Shaders
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyShaderModule() Succeeded For Fragment Shader\n", __func__);
    }

    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyShaderModule() Succeeded For Vertex Shader\n", __func__);
    }

    //* Destroy Uniform Buffer
    if (uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For uniformData.vkDeviceMemory\n", __func__);
    }

    if (uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For uniformData.vkBuffer\n", __func__);
    }

    //* Step - 14 of Vertex Buffer
    if (vertexData_position_1024x1024_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_1024x1024_graphics.vkDeviceMemory, NULL);
        vertexData_position_1024x1024_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vertexData_position_1024x1024_graphics.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position_1024x1024_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_1024x1024_graphics.vkBuffer, NULL);
        vertexData_position_1024x1024_graphics.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For vertexData_position_1024x1024_graphics.vkBuffer\n", __func__);
    }

    if (vertexData_position_512x512_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_512x512_graphics.vkDeviceMemory, NULL);
        vertexData_position_512x512_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vertexData_position_512x512_graphics.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position_512x512_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_512x512_graphics.vkBuffer, NULL);
        vertexData_position_512x512_graphics.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For vertexData_position_512x512_graphics.vkBuffer\n", __func__);
    }

    if (vertexData_position_256x256_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_256x256_graphics.vkDeviceMemory, NULL);
        vertexData_position_256x256_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vertexData_position_256x256_graphics.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position_256x256_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_256x256_graphics.vkBuffer, NULL);
        vertexData_position_256x256_graphics.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For vertexData_position_256x256_graphics.vkBuffer\n", __func__);
    }

    if (vertexData_position_128x128_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_128x128_graphics.vkDeviceMemory, NULL);
        vertexData_position_128x128_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vertexData_position_128x128_graphics.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position_128x128_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_128x128_graphics.vkBuffer, NULL);
        vertexData_position_128x128_graphics.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For vertexData_position_128x128_graphics.vkBuffer\n", __func__);
    }

    if (vertexData_position_64x64_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_64x64_graphics.vkDeviceMemory, NULL);
        vertexData_position_64x64_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vertexData_position_64x64_graphics.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position_64x64_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_64x64_graphics.vkBuffer, NULL);
        vertexData_position_64x64_graphics.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyBuffer() Succeeded For vertexData_position_64x64_graphics.vkBuffer\n", __func__);
    }

    //* Step - 5 of Command Buffer
    if (vkCommandBuffer_1024x1024_graphics_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_1024x1024_graphics_array[i]);

        free(vkCommandBuffer_1024x1024_graphics_array);
        vkCommandBuffer_1024x1024_graphics_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkCommandBuffer_1024x1024_graphics_array\n", __func__);
    }

    if (vkCommandBuffer_512x512_graphics_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_512x512_graphics_array[i]);

        free(vkCommandBuffer_512x512_graphics_array);
        vkCommandBuffer_512x512_graphics_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkCommandBuffer_512x512_graphics_array\n", __func__);
    }

    if (vkCommandBuffer_256x256_graphics_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_256x256_graphics_array[i]);

        free(vkCommandBuffer_256x256_graphics_array);
        vkCommandBuffer_256x256_graphics_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkCommandBuffer_256x256_graphics_array\n", __func__);
    }

    if (vkCommandBuffer_128x128_graphics_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_128x128_graphics_array[i]);

        free(vkCommandBuffer_128x128_graphics_array);
        vkCommandBuffer_128x128_graphics_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkCommandBuffer_128x128_graphics_array\n", __func__);
    }

    if (vkCommandBuffer_64x64_graphics_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_64x64_graphics_array[i]);

        free(vkCommandBuffer_64x64_graphics_array);
        vkCommandBuffer_64x64_graphics_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkCommandBuffer_64x64_graphics_array\n", __func__);
    }

    //* Step - 4 of Command Pool (Destroy Command Pool)
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyCommandPool() Succeeded\n", __func__);
    }

    //* Destroying Depth Image
    if (vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyImageView() Succeeded For vkImageView_depth\n", __func__);
    }

    if (vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyImage() Succeeded For vkImage_depth\n", __func__);
    }

    if (vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkFreeMemory() Succeeded For vkDeviceMemory_depth\n", __func__);
    }

    //* Step - 7, 8 of Swapchain Image and Image Views
    if (swapchainImageView_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyImageView() Succeeded\n", __func__);

        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For swapchainImageView_array\n", __func__);
    }

    //! No need to free swapchain images ->  Uncommenting causes the code to crash
    // for (uint32_t i = 0; i < swapchainImageCount; i++)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
    //     __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyImage() Succeeded\n", __func__);
    // } 

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For swapchainImage_array\n", __func__);
    }

    //* Step - 10 of Swapchain (Destroy Swapchain)
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroySwapchainKHR() Succeeded\n", __func__);
    }

    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyDevice() Succeeded\n", __func__);
    }

    //* No need to destroy device queue

    //* No need to destroy selected physical device

    //* Step - 5 of Presentation Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroySurfaceKHR() Succeeded\n", __func__);
    }

    if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }

    //* Step - 5 of Instance Creation
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyInstance() Succeeded\n", __func__);
    }

    return vkResult;
}

//! Definition of Vulkan Related Functions
VkResult createVulkanInstance(void)
{
    // Function Declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = fillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillInstanceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillInstanceExtensionNames() Succeeded\n", __func__);

    //! Fill Validation Layers
    if (bValidation == true)
    {
        vkResult = fillValidationLayerNames();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillValidationLayerNames() Failed : %d !!!\n", __func__, vkResult);
            return VK_ERROR_INITIALIZATION_FAILED;
        }      
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillValidationLayerNames() Succeeded\n", __func__);
    }

    //* Step - 2
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpSzAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpSzAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;

    //* Step - 3
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    if (bValidation == true)
    {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }
        
    //* Step - 4
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
    if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateInstance() Failed Due To Incompatible Driver : %d!!!\n", __func__, vkResult);
        return vkResult;
    } 
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateInstance() Failed Because Required Extension Is Not Present : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateInstance() Failed : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else 
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateInstance() Succeeded\n", __func__);

    //! Handling Validation Callbacks
    if (bValidation == true)
    {
        vkResult = createValidationCallbackFunction();
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createValidationCallbackFunction() Failed : %d !!!\n", __func__, vkResult);
            return VK_ERROR_INITIALIZATION_FAILED;
        }      
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createValidationCallbackFunction() Succeeded\n", __func__);
    }

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
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(instanceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **instanceExtensionNames_array = NULL;
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    if (instanceExtensionNames_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For instanceExtensionNames_array !!!\n", __func__);
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
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For instanceExtensionNames_array[%d] !!!\n", __func__, i);
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

        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Vulkan Instance Extension Name : %s\n", __func__, instanceExtensionNames_array[i]);
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
    VkBool32 debugReportExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
           
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_ANDROID_SURFACE_EXTENSION_NAME) == 0)
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        } 

        if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            debugReportExtensionFound = VK_TRUE;
            if (bValidation == true)
                enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            else
            {
                // Array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
            }            
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
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    if (win32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_ANDROID_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_ANDROID_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    if (debugReportExtensionFound == VK_FALSE)
    {
        if (bValidation == true)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VALIDATION ON : VK_EXT_DEBUG_REPORT_EXTENSION_NAME Extension Not Supported !!!\n", __func__);
            return vkResult;
        }
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VALIDATION OFF : VK_EXT_DEBUG_REPORT_EXTENSION_NAME Extension Not Supported !!!\n", __func__);
    }
    else
    {
        if (bValidation == true)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VALIDATION ON : VK_EXT_DEBUG_REPORT_EXTENSION_NAME Extension Supported\n", __func__);
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VALIDATION OFF : VK_EXT_DEBUG_REPORT_EXTENSION_NAME Extension Supported\n", __func__);
    }

    //* Step - 8
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Enabled Vulkan Instance Extension Name : %s\n", __func__, enabledInstanceExtensionNames_array[i]);

    return vkResult;
}

VkResult fillValidationLayerNames(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    uint32_t validationLayerCount = 0;
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    if (vkResult != VK_SUCCESS)  
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateInstanceLayerProperties() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateInstanceLayerProperties() Succeeded\n", __func__);

    VkLayerProperties *vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*)malloc(validationLayerCount * sizeof(VkLayerProperties));
    if (vkLayerProperties_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkLayerProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
    if (vkResult != VK_SUCCESS)  
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateInstanceLayerProperties() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateInstanceLayerProperties() Succeeded\n", __func__);

    char **validationLayerNames_array = NULL;
    validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
    if (validationLayerNames_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For validationLayerNames_array !!!\n", __func__);
        if (vkLayerProperties_array)
        {
            free(vkLayerProperties_array);
            vkLayerProperties_array = NULL;
        }
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkLayerProperties_array[i].layerName) + 1));
        if (validationLayerNames_array[i] == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For validationLayerNames_array[%d] !!!\n", __func__, i);
            if (validationLayerNames_array)
            {
                free(validationLayerNames_array);
                validationLayerNames_array = NULL;
            }
            if (vkLayerProperties_array)
            {
                free(vkLayerProperties_array);
                vkLayerProperties_array = NULL;
            }
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);

        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Vulkan Instance Layer Name : %s\n", __func__, validationLayerNames_array[i]);
    }

    if (vkLayerProperties_array)
    {
        free(vkLayerProperties_array);
        vkLayerProperties_array = NULL;
    }

    VkBool32 validationLayerFound = VK_FALSE;
    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            validationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    if (validationLayerNames_array)
    {
        for (uint32_t i = 0; i < validationLayerCount; i++)
        {
            free(validationLayerNames_array[i]);
            validationLayerNames_array[i] = NULL;
        }
        free(validationLayerNames_array);
        validationLayerNames_array = NULL;
    }

    if (validationLayerFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_LAYER_KHRONOS_validation Not Supported !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_LAYER_KHRONOS_validation Supported\n", __func__);

    for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Enabled Vulkan Validation Layer Name : %s\n", __func__, enabledValidationLayerNames_array[i]);

    return vkResult;
}

VkResult createValidationCallbackFunction(void)
{
    // Callback Declaration
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
        VkDebugReportFlagsEXT,
        VkDebugReportObjectTypeEXT,
        uint64_t,
        size_t,
        int32_t,
        const char*,
        const char*,
        void*
    );

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

    // Code
    
    //* Get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
    if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetInstanceProcAddr() Failed To Get Function Pointer For vkCreateDebugReportCallbackEXT !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetInstanceProcAddr() Succeeded To Get Function Pointer For vkCreateDebugReportCallbackEXT\n", __func__);

    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetInstanceProcAddr() Failed To Get Function Pointer For vkDestroyDebugReportCallbackEXT !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetInstanceProcAddr() Succeeded To Get Function Pointer For vkDestroyDebugReportCallbackEXT\n", __func__);

    //* Get the Vulkan Debug Report Callback Object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDebugReportCallbackEXT_fnptr() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDebugReportCallbackEXT_fnptr() Succeeded\n", __func__);

    return vkResult;
}

VkResult getSupportedSurface(void)
{
    // Code

    //* Step - 1
    VkAndroidSurfaceCreateInfoKHR vkAndroidSurfaceCreateInfoKHR;
    VkResult vkResult = VK_SUCCESS;

    //* Step - 2
    memset((void*)&vkAndroidSurfaceCreateInfoKHR, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));

    //* Step - 3
    vkAndroidSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    vkAndroidSurfaceCreateInfoKHR.pNext = NULL;
    vkAndroidSurfaceCreateInfoKHR.flags = 0;
    vkAndroidSurfaceCreateInfoKHR.window = androidNativeWindow;
    

    //* Step - 4
    vkResult = vkCreateAndroidSurfaceKHR(vkInstance, &vkAndroidSurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateAndroidSurfaceKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateAndroidSurfaceKHR() Succeeded\n", __func__);

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
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() Call 1 => vkEnumeratePhysicalDevices() Succeeded\n", __func__); 
    else if (physicalDeviceCount == 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkEnumeratePhysicalDevices() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() Call 1 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //* Step - 3
    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (vkPhysicalDevice_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkPhysicalDevice_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 4
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (vkResult != VK_SUCCESS)
    {   
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() Call 2 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
       __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() Call 2 => vkEnumeratePhysicalDevices() Succeeded\n", __func__);

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
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkQueueFamilyProperties_array !!!\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        //* Step - 5.4
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        //* Step - 5.5
        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
        if (isQueueSurfaceSupported_array == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For isQueueSurfaceSupported_array\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        //* Step - 5.6
        for (uint32_t j = 0; j < queueCount; j++)
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);

        //* Step - 5.7
        for (uint32_t j = 0; j < queueCount; j++)
        {
            if (vkQueueFamilyProperties_array[j].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
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
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For isQueueSurfaceSupported_array\n", __func__);
            isQueueSurfaceSupported_array = NULL;
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkQueueFamilyProperties_array\n", __func__);
            vkQueueFamilyProperties_array = NULL;
        }

        //* Step - 5.9
        if (bFound == VK_TRUE)
            break;
        
    }

    //* Step - 5.10
    if (bFound == VK_TRUE)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Succeeded To Obtain Graphics Supported Physical Device\n", __func__);

    //* Step - 6
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Obtain Graphics Supported Physical Device !!!\n", __func__);
        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
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
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

    //* Step - 10
    if (vkPhysicalDeviceFeatures.tessellationShader == VK_TRUE)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Selected Physical Device Supports Tessellation Shader\n", __func__);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Selected Physical Device Does Not Support Tessellation Shader !!!\n", __func__);

    if (vkPhysicalDeviceFeatures.geometryShader == VK_TRUE)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Selected Physical Device Supports Geometry Shader\n", __func__);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Selected Physical Device Does Not Support Geometry Shader !!!\n", __func__);

    return vkResult;
}

VkResult printVkInfo(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "\nVULKAN INFORMATION\n");
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "------------------------------------------------------------------------------------------------");
    
    //* Step - 3.1
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "\nDevice Number : %d\n", i);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "*******************************************************\n");
        
        //* Step - 3.2
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

        //* Step - 3.3
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vulkan API Version : %u.%u.%u\n", majorVersion, minorVersion, patchVersion);

        //* Step - 3.4
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Name : %s\n", vkPhysicalDeviceProperties.deviceName);

        //* Step - 3.5
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : Integrated GPU (iGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : Discrete GPU (dGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : Virtual GPU (vGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : CPU\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : Other\n");
            break;

            default:
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device Type : UNKNOWN\n");
            break;
        }

        //* Step - 3.6
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Device ID : 0x%4x\n", vkPhysicalDeviceProperties.deviceID);

        //* Step - 3.7
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vendor ID : 0x%4x\n", vkPhysicalDeviceProperties.vendorID);

        switch(vkPhysicalDeviceProperties.vendorID)
        {
            case 0x10DE: __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vendor Name : NVIDIA\n"); break;
            case 0x1002: __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vendor Name : AMD\n"); break;
            case 0x8086: __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vendor Name : Intel\n"); break;
            default: __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vendor Name : Unknown (0x%4x)\n", vkPhysicalDeviceProperties.vendorID);
        }

        //* Additional Properties
        uint32_t queueCount = UINT32_MAX;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "\nNo. of Queue Families: %d\n", queueCount);

        VkQueueFamilyProperties* vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(queueCount * sizeof(VkQueueFamilyProperties));
        if (vkQueueFamilyProperties_array == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkQueueFamilyProperties_array !!!\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
        if (isQueueSurfaceSupported_array == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For isQueueSurfaceSupported_array\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        for (uint32_t j = 0; j < queueCount; j++)
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);

        for (uint32_t j = 0; j < queueCount; j++)
        {

            __android_log_print(ANDROID_LOG_INFO, "ADN:", "\nQueue Family : %d\n", j);
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "****************************************\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Graphics : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Graphics : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Compute : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Compute : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Transfer Operations : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Transfer Operations : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Video Encoding : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Video Encoding : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Video Decoding : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Video Decoding : No\n");

            if (isQueueSurfaceSupported_array[j] == VK_TRUE)
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Presentation : Yes\n");
            else
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Supports Presentation : No\n");
                
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "****************************************\n\n");
        }

        if (isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
        }

        __android_log_print(ANDROID_LOG_INFO, "ADN:", "*******************************************************\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "ADN:", "------------------------------------------------------------------------------------------------\n\n");

    //* Step - 3.8
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
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
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **deviceExtensionNames_array = NULL;
    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    if (deviceExtensionNames_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For deviceExtensionNames_array !!!\n", __func__);
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
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For deviceExtensionNames_array[%d] !!!\n", __func__, i);
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

        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Vulkan Device Extension Name : %s\n", __func__, deviceExtensionNames_array[i]);
    }

    __android_log_print(ANDROID_LOG_INFO, "ADN:", "\n------------------------------------------------------------------------------------------------\n");
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Vulkan Device Extension Count : %d\n", __func__, deviceExtensionCount);
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "------------------------------------------------------------------------------------------------\n\n");

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
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Found\n", __func__);

    //* Step - 8
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Enabled Vulkan Device Extension Name : %s\n", __func__, enabledDeviceExtensionNames_array[i]);

    return vkResult;

}

VkResult createVulkanDevice(void)
{
    // Function Declaration
    VkResult fillDeviceExtensionNames(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    float queuePriorities[1] = { 1.0f };

    // Code

    //* Step - 1
    vkResult = fillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillDeviceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => fillDeviceExtensionNames() Succeeded\n", __func__);

    
    //* Step - 2
    //! Newly Added Code
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    //* Deprecated in Vulkan Spec
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;

    //* Step - 3
    vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDevice() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }      
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDevice() Succeeded\n", __func__);

    return vkResult;
}

void getDeviceQueue(void)
{
    // Code
    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if (vkQueue == VK_NULL_HANDLE)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetDeviceQueue() returned NULL for vkQueue !!!\n", __func__);
        return;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetDeviceQueue() Succeeded ...\n", __func__);
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t formatCount = 0;
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (formatCount == 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

    //* Step - 2
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    if (vkSurfaceFormatKHR_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkSurfaceFormatKHR_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

    //* Step - 4
    if (formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
        vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    }

    //* Step - 5
    if (vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkSurfaceFormatKHR_array\n", __func__);
    }

    return vkResult;
}

VkResult getPhysicalDevicePresentMode(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t presentModeCount = 0;
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (presentModeCount == 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Succeeded\n", __func__);

    //* Step - 2
    VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    if (vkPresentModeKHR_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkPresentModeKHR_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetPhysicalDeviceSurfacePresentModesKHR() Succeeded\n", __func__);


    //* Step - 4
    for (uint32_t i = 0; i < presentModeCount; i++)
    {
        if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "\n------------------------------------------------------------------------------------------------\n");
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vulkan Physical Device Present Mode : VK_PRESENT_MODE_MAILBOX_KHR");
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "\n------------------------------------------------------------------------------------------------\n\n");
            break;
        }
    }

    if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "\n------------------------------------------------------------------------------------------------\n");
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Vulkan Physical Device Present Mode : VK_PRESENT_MODE_FIFO_KHR");
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "\n------------------------------------------------------------------------------------------------\n\n");
    }
        

    //* Step - 5
    if (vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For vkPresentModeKHR_array\n", __func__);
    }

    return vkResult;
}

VkResult createSwapchain(VkBool32 vsync)
{
    // Function Declarations
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDeviceSurfaceFormatAndColorSpace() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDeviceSurfaceFormatAndColorSpace() Succeeded\n", __func__);

    //* Step - 2
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Succeeded\n", __func__);

    //* Step - 3
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumberOfSwapchainImages = 0;

    if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    else
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;

    //* Step - 4
    memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => [If Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // If surface size is already defined, then swapchain image size must match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_swapchain.width = glm::max(
            vkSurfaceCapabilitiesKHR.minImageExtent.width, 
            glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)
        );

        vkExtent2D_swapchain.height = glm::max(
            vkSurfaceCapabilitiesKHR.minImageExtent.height,
            glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)
        );

        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => [Else Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }

    //* Step - 5
    //! VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT => Mandatory
    //! VK_IMAGE_USAGE_TRANSFER_SRC_BIT => Optional (Useful for Texture, FBO, Compute)
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    //* Step - 6
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) //* Check For Identity Matrix
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;

    //* Step - 7
    vkResult = getPhysicalDevicePresentMode();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDevicePresentMode() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getPhysicalDevicePresentMode() Succeeded\n", __func__);

    //* Step - 8
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateSwapchainKHR() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateSwapchainKHR() Succeeded\n", __func__);


    return VK_SUCCESS;
}


VkResult createImagesAndImageViews(void)
{
    // Function Declarations
    VkResult getSupportedDepthFormat(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetSwapchainImagesKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (swapchainImageCount == 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetSwapchainImagesKHR() Returned 0 Images !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 1 : vkGetSwapchainImagesKHR() => Swapchain Image Count = %d\n", __func__, swapchainImageCount);


    //* Step - 2
    swapchainImage_array = (VkImage*)malloc(swapchainImageCount * sizeof(VkImage));
    if (swapchainImage_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For swapchainImage_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetSwapchainImagesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Call 2 : vkGetSwapchainImagesKHR() Succeeded\n", __func__);


    //* Step - 4
    swapchainImageView_array = (VkImageView*)malloc(swapchainImageCount * sizeof(VkImageView));
    if (swapchainImageView_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For swapchainImageView_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 5
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    //* Step - 6
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[i];
        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImageView() Failed For Index : %d, Error Code : %d !!!\n", __func__, i, vkResult);
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImageView() Succeeded For Index : %d\n", __func__, i);
    }

    //! For Depth Image
    vkResult = getSupportedDepthFormat();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getSupportedDepthFormat() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => getSupportedDepthFormat() Succeded\n", __func__);

    //* For Depth Image, initialize VkImageCreateInfo
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = vkFormat_depth;
    vkImageCreateInfo.extent.width = winWidth;
    vkImageCreateInfo.extent.height = winHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_depth);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImage() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImage() Succeeded\n", __func__);

    //! Memory Requirements For Depth Image
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vkDevice, vkImage_depth, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Failed For Depth : %d !!!\n", __func__, vkResult);
        return vkResult;
    }     
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Succeeded For Depth\n", __func__);

    vkResult = vkBindImageMemory(vkDevice, vkImage_depth, vkDeviceMemory_depth, 0);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindImageMemory() Failed For Depth : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindImageMemory() Succeeded For Depth\n", __func__);

    //! Create Image View For Above Depth Image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_depth;    //! Added here, as previously we had swapchain images, but here we are creating a new depth image

    //* Step - 6
    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_depth);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImageView() Failed For Depth : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateImageView() Succeeded For Depth\n", __func__);

    return vkResult;
}

VkResult getSupportedDepthFormat(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    
    VkFormat vkFormat_depth_array[] = 
    {
        //* Descending Order
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };
    
    // Code
    for (uint32_t i = 0; i < (sizeof(vkFormat_depth_array) / sizeof(vkFormat_depth_array[0])); i++)
    {
        VkFormatProperties vkFormatProperties;
        memset((void*)&vkFormatProperties, 0, sizeof(VkFormatProperties));
        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_selected, vkFormat_depth_array[i], &vkFormatProperties);

        if (vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            vkFormat_depth = vkFormat_depth_array[i];
            vkResult = VK_SUCCESS;
            break;
        }    
    }

    return vkResult;
}

VkResult createCommandPool(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
    memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateCommandPool() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateCommandPool() Succeeded\n", __func__);

    return vkResult;
}

VkResult createCommandBuffers(VkCommandBuffer** pPvkCommandBuffer_array)
{
    // Variable Declarations
    VkCommandBuffer *vkCommandBuffer_array = NULL;
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; 
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    //* Step - 2
    vkCommandBuffer_array = (VkCommandBuffer*)malloc(swapchainImageCount * sizeof(VkCommandBuffer));
    if (vkCommandBuffer_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkCommandBuffer_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateCommandBuffers() Failed For Index : %d, Error Code : %d !!!\n", __func__, i, vkResult);
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateCommandBuffers() Succeeded For Index : %d\n", __func__, i);
    }

    *pPvkCommandBuffer_array = vkCommandBuffer_array;

    return vkResult;
}


VkResult createVertexBuffer(uint32_t meshWidth, uint32_t meshHeight, VertexData* pVertexData)
{
    // Variable Declarations
    VertexData vertexData_position;
    VkResult vkResult = VK_SUCCESS;

    // Code
    
    //! Vertex Position
    //! -------------------------------------------------------------------------------------------------------------------------------------
    VkDeviceSize bufferSize = meshWidth * meshHeight * 4 * sizeof(float);

    memset((void*)&vertexData_position, 0, sizeof(VertexData));

    //* Step - 5
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.flags = 0;   //! Valid Flags are used in sparse(scattered) buffers
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.size = bufferSize;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    //* Step - 6
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Failed For Vertex Position Buffer  : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Succeeded For Vertex Position Buffer\n", __func__);
    
    //* Step - 7
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    //* Step - 8
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
    
    //* Step - 8.1
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        //* Step - 8.2
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            //* Step - 8.3
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                //* Step - 8.4
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        //* Step - 8.5
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //* Step - 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Failed For Vertex Position Buffer : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Succeeded For Vertex Position Buffer\n", __func__);

    //* Step - 10
    //! Binds Vulkan Device Memory Object Handle with the Vulkan Buffer Object Handle
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Failed For Vertex Position Buffer : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Succeeded For Vertex Position Buffer\n", __func__);

    //* Step - 11
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For Vertex Position Buffer : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Succeeded For Vertex Position Buffer\n", __func__);

    if (meshWidth == 64 && meshHeight == 64)
        memcpy(data, position_64_graphics, bufferSize);
    else if (meshWidth == 128 && meshHeight == 128)
        memcpy(data, position_128_graphics, bufferSize);
    else if (meshWidth == 256 && meshHeight == 256)
        memcpy(data, position_256_graphics, bufferSize);
    else if (meshWidth == 512 && meshHeight == 512)
        memcpy(data, position_512_graphics, bufferSize);
    else if (meshWidth == 1024 && meshHeight == 1024)
        memcpy(data, position_1024_graphics, bufferSize);

    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);
    //! -------------------------------------------------------------------------------------------------------------------------------------
    
    *pVertexData = vertexData_position;

    return vkResult;
}

void initializeSinewaveArrays(uint32_t meshWidth, uint32_t meshHeight)
{
    // Code
    if (meshWidth == 64 && meshHeight == 64)
    {
        for (unsigned int i = 0; i < meshWidth; i++)
        {
            for (unsigned int j = 0; j < meshHeight; j++)
            {
                for (unsigned int k = 0; k < 4; k++)
                {
                    position_64_graphics[i][j][k] = 0;
                }
            }
        }
    }
    
    else if (meshWidth == 128 && meshHeight == 128)
    {
        for (unsigned int i = 0; i < meshWidth; i++)
        {
            for (unsigned int j = 0; j < meshHeight; j++)
            {
                for (unsigned int k = 0; k < 4; k++)
                {
                    position_128_graphics[i][j][k] = 0;
                }
            }
        }
    }
    
    else if (meshWidth == 256 && meshHeight == 256)
    {
        for (unsigned int i = 0; i < meshWidth; i++)
        {
            for (unsigned int j = 0; j < meshHeight; j++)
            {
                for (unsigned int k = 0; k < 4; k++)
                {
                    position_256_graphics[i][j][k] = 0;
                }
            }
        }
    }

    else if (meshWidth == 512 && meshHeight == 512)
    {
        for (unsigned int i = 0; i < meshWidth; i++)
        {
            for (unsigned int j = 0; j < meshHeight; j++)
            {
                for (unsigned int k = 0; k < 4; k++)
                {
                    position_512_graphics[i][j][k] = 0;
                }
            }
        }
    }

    else if (meshWidth == 1024 && meshHeight == 1024)
    {
        for (unsigned int i = 0; i < meshWidth; i++)
        {
            for (unsigned int j = 0; j < meshHeight; j++)
            {
                for (unsigned int k = 0; k < 4; k++)
                {
                    position_1024_graphics[i][j][k] = 0;
                }
            }
        }
    }
    
}

VkResult createUniformBuffer(void)
{
    // Function Declarations
    VkResult updateUniformBuffer(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.size = sizeof(Host_UniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    memset((void*)&uniformData, 0, sizeof(UniformData));

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Succeeded For Uniform Data\n", __func__);
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }     
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Succeeded For Uniform Data\n", __func__);

    vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Succeeded For Uniform Data\n", __func__);

    vkResult = updateUniformBuffer();
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => updateUniformBuffer() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => updateUniformBuffer() Succeeded\n", __func__);


    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    Host_UniformData host_uniformData;
    memset((void*)&host_uniformData, 0, sizeof(Host_UniformData));

    //! Update Matrices
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    glm::mat4 modelViewMatrix = glm::mat4(1.0f);
    glm::mat4 modelViewProjectionMatrix = glm::mat4(1.0f);

    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    modelViewMatrix = translationMatrix;
    
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);
    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float)winWidth / (float)winHeight,
        0.1f,
        100.0f
    );
    //! 2D Matrix with Column Major (Like OpenGL)
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f);
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    host_uniformData.mvpMatrix = modelViewProjectionMatrix;

    //* Color Update
    switch (selectedColor)
    {
        // Background - White, Foreground - Black
        case 'K': 
        {
            host_uniformData.color = glm::vec4(0.0, 0.0, 0.0, 1.0); 
        }
        break;

        // For all other colors - Black background
        case 'W': host_uniformData.color = glm::vec4(1.0, 1.0, 1.0, 1.0); break;    // White
        case 'R': host_uniformData.color = glm::vec4(1.0, 0.0, 0.0, 1.0); break;    // Red
        case 'G': host_uniformData.color = glm::vec4(0.0, 1.0, 0.0, 1.0); break;    // Green
        case 'B': host_uniformData.color = glm::vec4(0.0, 0.0, 1.0, 1.0); break;    // Blue
        case 'O': host_uniformData.color = glm::vec4(1.0, 0.5, 0.0, 1.0); break;    // Orange
        case 'C': host_uniformData.color = glm::vec4(0.0, 1.0, 1.0, 1.0); break;    // Cyan
        case 'M': host_uniformData.color = glm::vec4(1.0, 0.0, 1.0, 1.0); break;    // Magenta
        case 'Y': host_uniformData.color = glm::vec4(1.0, 1.0, 0.0, 1.0); break;    // Yellow

        default:
            host_uniformData.color = glm::vec4(1.0, 0.5, 0.0, 1.0);   // Orange
        break;
    }

    //! Map Uniform Buffer
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(Host_UniformData), 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For Uniform Buffer : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //! Copy the data to the mapped buffer (present on device memory)
    memcpy(data, &host_uniformData, sizeof(Host_UniformData));

    //! Unmap memory
    vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

    return vkResult;
}

VkResult createShaders(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    AAsset* shaderAsset = NULL;

    //! Vertex Shader
    //! ---------------------------------------------------------------------------------------------------------------------------
    //* Step - 6
    const char* szFileName = "Shader.vert.spv";
    shaderAsset = AAssetManager_open(androidAssetManager, szFileName, AASSET_MODE_BUFFER);
    if (shaderAsset == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Open SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Succeeded In Opening SPIR-V Shader File : %s\n", __func__, szFileName);

    size_t size = AAsset_getLength(shaderAsset);

    char* shaderData = (char*)malloc(size * sizeof(char));
    if (shaderData == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For shaderData !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    size_t retVal = AAsset_read(shaderAsset, shaderData, size);
    if (retVal < 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Read From SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Successfully Read Shader From SPIR-V Shader File : %s\n", __func__, szFileName);
    
    if (shaderAsset)
    {
        AAsset_close(shaderAsset); 
        shaderAsset = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Closed SPIR-V File : %s\n", __func__, szFileName);
    }

    //* Step - 7
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //! Reserved, must be 0
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;
    vkShaderModuleCreateInfo.codeSize = size;

    //* Step - 8
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Failed For Vertex Shader : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Succeeded For Vertex Shader\n", __func__);

    //* Step - 9
    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For shaderData\n", __func__);
    }

    __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Vertex Shader Module Successfully Created\n", __func__);
    //! ---------------------------------------------------------------------------------------------------------------------------

    //! Fragment Shader
    //! ---------------------------------------------------------------------------------------------------------------------------
    szFileName = "Shader.frag.spv";
    shaderAsset = AAssetManager_open(androidAssetManager, szFileName, AASSET_MODE_BUFFER);
    if (shaderAsset == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Open SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Succeeded In Opening SPIR-V Shader File : %s\n", __func__, szFileName);

    size = AAsset_getLength(shaderAsset);

    shaderData = (char*)malloc(size * sizeof(char));
    if (shaderData == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For shaderData !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    retVal = AAsset_read(shaderAsset, shaderData, size);
    if (retVal < 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Read From SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Successfully Read Shader From SPIR-V Shader File : %s\n", __func__, szFileName);
    
    if (shaderAsset)
    {
        AAsset_close(shaderAsset); 
        shaderAsset = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Closed SPIR-V File : %s\n", __func__, szFileName);
    }

    //* Step - 7
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //! Reserved, must be 0
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;
    vkShaderModuleCreateInfo.codeSize = size;

    //* Step - 8
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Failed For Fragment Shader : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Succeeded For Fragment Shader\n", __func__);

    //* Step - 9
    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For shaderData\n", __func__);
    }

    __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Fragment Shader Module Successfully Created\n", __func__);
    //! ---------------------------------------------------------------------------------------------------------------------------

    return vkResult;
}

VkResult createDescriptorSetLayout(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //! Initialize VkDescriptorSetLayoutBinding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
    memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding.binding = 0;
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;

    //* Step - 3
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;   //! An integer value where you want to bind descriptor set
    vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;

    //* Step - 4
    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorSetLayout() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorSetLayout() Succeeded\n", __func__);

    return vkResult;
}

VkResult createPipelineLayout(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 3
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    //* Step - 4
    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineLayout() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineLayout() Succeeded\n", __func__);

    return vkResult;
}

VkResult createDescriptorPool(void)
{
    // Variable Declarations
    VkResult vkResult;

    // Code

    //* Vulkan expects decriptor pool size before creating actual descriptor pool
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));

    // Graphics UBO
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1;

    // Compute SSBO
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1;
   
    //* Create the pool
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
    vkDescriptorPoolCreateInfo.maxSets = 2;     //! 1 - GFX, 2 - Compute

    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorPool() Failed : %d !!!\n", __func__, vkResult);  
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorPool() Succeeded\n", __func__);

    return vkResult;
}

VkResult createDescriptorSet(void)
{
    // Variable Declarations
    VkResult vkResult;

    // Code

    //* Initialize DescriptorSetAllocationInfo
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateDescriptorSets() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }  
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateDescriptorSets() Succeeded\n", __func__);
    
    //* Describe whether we want buffer as uniform or image as uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(Host_UniformData);

    /* Update above descriptor set directly to the shader
    There are 2 ways :-
        1) Writing directly to the shader
        2) Copying from one shader to another shader
    */
    VkWriteDescriptorSet vkWriteDescriptorSet;
    memset((void*)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));
    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

    return vkResult;
}

VkResult createRenderPass(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    VkAttachmentDescription vkAttachmentDescription_array[2];   //! Size changed to 2 to accomodate depth
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    //! Color Attachment (Graphics Pipeline)
    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT; //* No MSAA
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //! Depth Attachment
    vkAttachmentDescription_array[1].flags = 0;
    vkAttachmentDescription_array[1].format = vkFormat_depth;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //* Step - 2
    //! Color Attachment Reference
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_color.attachment = 0;   //* 0 specifies 0th index in above array
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //! Depth Attachment Reference
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_depth.attachment = 1;   //* 1 specifies 1st index in above array
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //* Step - 3
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1;  //! This should be the count of vkAttachmentReference used for color
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
    vkSubpassDescription.pPreserveAttachments = NULL;
    vkSubpassDescription.pResolveAttachments = NULL;

    //* Step - 4
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    //* Step - 5
    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateRenderPass() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateRenderPass() Succeeded\n", __func__);

    return vkResult;
}

VkResult createPipeline(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Code

    //! Vertex Input State
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
    
    //! Position
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 4; 
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
    
    //! Position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
    
    //! Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    //! Rasterization State
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    //! Color Blend State
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

    //! Viewport Scissor State
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1;    //* We can specify multiple viewports here
    vkPipelineViewportStateCreateInfo.scissorCount = 1;

    //! Viewport Info     
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0;
    vkViewport.y = 0;
    vkViewport.width = (float)vkExtent2D_swapchain.width;
    vkViewport.height = (float)vkExtent2D_swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
    
    //! Scissor Info
    memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
    vkRect2D_scissor.offset.x = 0;
    vkRect2D_scissor.offset.y = 0;
    vkRect2D_scissor.extent.width = vkExtent2D_swapchain.width;
    vkRect2D_scissor.extent.height = vkExtent2D_swapchain.height;
   
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;

    //! Depth Stencil State !//
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilCreateInfo;
    memset((void*)&vkPipelineDepthStencilCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilCreateInfo.flags = 0;
    vkPipelineDepthStencilCreateInfo.pNext = NULL;
    vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    vkPipelineDepthStencilCreateInfo.front = vkPipelineDepthStencilCreateInfo.back;

    //! Dynamic State !//

    //! Multi-Sample State
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //! Shader Stage State
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
    
    //* Vertex Shader
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

    //* Fragment Shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

    //! Tessellation State !//

    //! As pipelines are created from pipeline caches, we will create VkPipelineCache Object
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    
    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineCache() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineCache() Succeeded\n", __func__);

    //! Create actual Graphics Pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0;
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateGraphicsPipelines() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateGraphicsPipelines() Succeeded\n", __func__);

    //* Destroy Pipeline Cache
    if (vkPipelineCache)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyPipelineCache() Succeeded\n", __func__);
    }

    return vkResult;
}


VkResult createFramebuffers(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    if (vkFramebuffer_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkFramebuffer_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 2
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        //* Step - 3
        VkImageView vkImageView_attachments_array[2];
        memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

        //* Step - 4
        VkFramebufferCreateInfo vkFramebufferCreateInfo;
        memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.pNext = NULL;
        vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
        vkFramebufferCreateInfo.renderPass = vkRenderPass;
        vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
        vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
        vkFramebufferCreateInfo.layers = 1;

        vkImageView_attachments_array[0] = swapchainImageView_array[i];
        vkImageView_attachments_array[1] = vkImageView_depth;

        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateFramebuffer() Failed For Index : %d, Reason : %d !!!\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
    }

    return vkResult;
}

VkResult createSemaphores(void)
{
    // Code
    VkResult vkResult = VK_SUCCESS;

    //* Step - 2
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.flags = 0;    //! Must Be 0 (Reserved)
    vkSemaphoreCreateInfo.pNext = NULL;

    //* Step - 3
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backBuffer);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateSemaphore() Failed For vkSemaphore_backBuffer : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateSemaphore() Failed For vkSemaphore_renderComplete : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    return vkResult;
}

VkResult createFences(void)
{
    // Code
    VkResult vkResult = VK_SUCCESS;

    //* Step - 4
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //* Step - 5
    vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount);
    if (vkFence_array == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For vkFence_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 6
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateFence() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        else
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateFence() Succeeded For Index : %d\n", __func__, i);
    }

    return vkResult;
}

VkResult buildCommandBuffers(void)
{
    // Function Declarations
    VkResult prepareSineWaveForCPU(uint32_t, uint32_t, float);

    // Variable Declarations
    VkCommandBuffer *vkCommandBuffer_array = NULL;
    VkResult vkResult = VK_SUCCESS;

    // Code

    if (bMesh64Chosen)
    {
        if (!bUseCompute)
        {
            vkResult = prepareSineWaveForCPU(64, 64, fAnimationSpeed);
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => prepareSineWaveForCPU() Failed For 64x64 : %d\n", __func__, vkResult);
                vkResult = VK_ERROR_INITIALIZATION_FAILED;
                return vkResult;
            }
        }
        vkCommandBuffer_array = vkCommandBuffer_64x64_graphics_array;
    }
    else if (bMesh128Chosen)
    {
        if (!bUseCompute)
        {
            vkResult = prepareSineWaveForCPU(128, 128, fAnimationSpeed);
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => prepareSineWaveForCPU() Failed For 128x128 : %d\n", __func__, vkResult);
                vkResult = VK_ERROR_INITIALIZATION_FAILED;
                return vkResult;
            }
            vkCommandBuffer_array = vkCommandBuffer_128x128_graphics_array;
        }
    }
    else if (bMesh256Chosen)
    {
        if (!bUseCompute)
        {
            vkResult = prepareSineWaveForCPU(256, 256, fAnimationSpeed);
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => prepareSineWaveForCPU() Failed For 256x256 : %d\n", __func__, vkResult);
                vkResult = VK_ERROR_INITIALIZATION_FAILED;
                return vkResult;
            }
            vkCommandBuffer_array = vkCommandBuffer_256x256_graphics_array;
        }
    }   
    else if (bMesh512Chosen)
    {
        if (!bUseCompute)
        {
            vkResult = prepareSineWaveForCPU(512, 512, fAnimationSpeed);
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => prepareSineWaveForCPU() Failed For 512x512 : %d\n", __func__, vkResult);
                vkResult = VK_ERROR_INITIALIZATION_FAILED;
                return vkResult;
            }
        }
        vkCommandBuffer_array = vkCommandBuffer_512x512_graphics_array;
    }
    else if (bMesh1024Chosen)
    {
        if (!bUseCompute)
        {
            vkResult = prepareSineWaveForCPU(2048, 2048, fAnimationSpeed);
            if (vkResult != VK_SUCCESS)
            {
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => prepareSineWaveForCPU() Failed For 1024x1024 : %d\n", __func__, vkResult);
                vkResult = VK_ERROR_INITIALIZATION_FAILED;
                return vkResult;
            }
        }
        vkCommandBuffer_array = vkCommandBuffer_1024x1024_graphics_array;
    }
    
    //! Loop per swapchain image
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        //* Step - 1 => Reset Command Buffer
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);   //! 0 specifies not to release the resources
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkResetCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }

        //* Step - 2
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;     //! 0 specifies that we will use only the primary command buffer, and not going to use this command buffer simultaneously between multiple threads

        //* Step - 3
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBeginCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }

        //* Step - 4 => Set Clear Value
        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));
        if (selectedColor == 'K')
        {
            VkClearColorValue vkClearColorValue_white = { 1.0f, 1.0f, 1.0f, 1.0f };
            vkClearValue_array[0].color = vkClearColorValue_white;
        }
        else
            vkClearValue_array[0].color = vkClearColorValue;

        vkClearValue_array[1].depthStencil = vkClearDepthStencilValue;

        //* Pipeline Barrier
        if (bUseCompute)
        {
            VkBufferMemoryBarrier vkBufferMemoryBarrier;
            memset((void*)&vkBufferMemoryBarrier, 0, sizeof(VkBufferMemoryBarrier));
            vkBufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            vkBufferMemoryBarrier.pNext = NULL;
            vkBufferMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            vkBufferMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            vkBufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBufferMemoryBarrier.buffer = computeBuffer.vkBuffer;
            vkBufferMemoryBarrier.offset = 0;
            vkBufferMemoryBarrier.size = VK_WHOLE_SIZE;

            vkCmdPipelineBarrier(
                vkCommandBuffer_array[i],
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                0,
                0,
                NULL,
                1,
                &vkBufferMemoryBarrier,
                0,
                NULL
            );

        }

        //* Step - 5
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[i];
        
        //* Step - 6
        vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            //! Bind with Pipeline
            vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

            //! Bind the Descriptor Set to the Pipeline
            vkCmdBindDescriptorSets(
                vkCommandBuffer_array[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                vkPipelineLayout,
                0,
                1,
                &vkDescriptorSet,
                0,
                NULL
            );

            if (bMesh64Chosen)
            {
                //! Bind with Vertex Position Buffer
                VkDeviceSize vkDeviceSize_offset_position[1];
                memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(
                    vkCommandBuffer_array[i], 
                    0, 
                    1, 
                    &vertexData_position_64x64_graphics.vkBuffer, 
                    vkDeviceSize_offset_position
                );

                //! Vulkan Drawing Function
                vkCmdDraw(vkCommandBuffer_array[i], 64 * 64, 1, 0, 0);
            }
            else if (bMesh128Chosen)
            {
                //! Bind with Vertex Position Buffer
                VkDeviceSize vkDeviceSize_offset_position[1];
                memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(
                    vkCommandBuffer_array[i], 
                    0, 
                    1, 
                    &vertexData_position_128x128_graphics.vkBuffer, 
                    vkDeviceSize_offset_position
                );

                //! Vulkan Drawing Function
                vkCmdDraw(vkCommandBuffer_array[i], 128 * 128, 1, 0, 0);
            }
            else if (bMesh256Chosen)
            {
                //! Bind with Vertex Position Buffer
                VkDeviceSize vkDeviceSize_offset_position[1];
                memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(
                    vkCommandBuffer_array[i], 
                    0, 
                    1, 
                    &vertexData_position_256x256_graphics.vkBuffer, 
                    vkDeviceSize_offset_position
                );

                //! Vulkan Drawing Function
                vkCmdDraw(vkCommandBuffer_array[i], 256 * 256, 1, 0, 0);
            }
            else if (bMesh512Chosen)
            {
                //! Bind with Vertex Position Buffer
                VkDeviceSize vkDeviceSize_offset_position[1];
                memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(
                    vkCommandBuffer_array[i], 
                    0, 
                    1, 
                    &vertexData_position_512x512_graphics.vkBuffer, 
                    vkDeviceSize_offset_position
                );

                //! Vulkan Drawing Function
                vkCmdDraw(vkCommandBuffer_array[i], 512 * 512, 1, 0, 0);
            }
            else if (bMesh1024Chosen)
            {
                //! Bind with Vertex Position Buffer
                VkDeviceSize vkDeviceSize_offset_position[1];
                memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));

                if (bUseCompute)
                {
                    vkCmdBindVertexBuffers(
                        vkCommandBuffer_array[i], 
                        0, 
                        1, 
                        &computeBuffer.vkBuffer, 
                        vkDeviceSize_offset_position
                    );
                }
                else
                {
                    vkCmdBindVertexBuffers(
                        vkCommandBuffer_array[i], 
                        0, 
                        1, 
                        &vertexData_position_1024x1024_graphics.vkBuffer, 
                        vkDeviceSize_offset_position
                    );
                }
                

                //! Vulkan Drawing Function
                vkCmdDraw(vkCommandBuffer_array[i], 2048 * 2048, 1, 0, 0);
            
            }
        }
        //* Step - 7
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);

        //* Step - 8
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkEndCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
    }

    return vkResult;
}

VkResult prepareSineWaveForCPU(uint32_t meshWidth, uint32_t meshHeight, float animationTime)
{
    // Function Declarations
    void populateSineWaveArraysForCPU(uint32_t, uint32_t, float);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    void *data = NULL;

    VkDeviceSize bufferSize = meshWidth * meshHeight * 4 * sizeof(float);

    // Code

    // Map the buffer
    if (meshWidth == 64 && meshHeight == 64)
    {
        populateSineWaveArraysForCPU(64, 64, animationTime);

        vkResult = vkMapMemory(vkDevice, vertexData_position_64x64_graphics.vkDeviceMemory, 0, bufferSize, 0, &data);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For vertexData_position_64x64_graphics : %d\n", __func__, vkResult);

        memcpy(data, position_64_graphics, bufferSize);

        vkUnmapMemory(vkDevice, vertexData_position_64x64_graphics.vkDeviceMemory);
    }

    else if (meshWidth == 128 && meshHeight == 128)
    {
        populateSineWaveArraysForCPU(128, 128, animationTime);

        vkResult = vkMapMemory(vkDevice, vertexData_position_128x128_graphics.vkDeviceMemory, 0, bufferSize, 0, &data);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For vertexData_position_128x128_graphics : %d\n", __func__, vkResult);

        memcpy(data, position_128_graphics, bufferSize);

        vkUnmapMemory(vkDevice, vertexData_position_128x128_graphics.vkDeviceMemory);
    }

    else if (meshWidth == 256 && meshHeight == 256)
    {
        populateSineWaveArraysForCPU(256, 256, animationTime);

        vkResult = vkMapMemory(vkDevice, vertexData_position_256x256_graphics.vkDeviceMemory, 0, bufferSize, 0, &data);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For vertexData_position_256x256_graphics : %d\n", __func__, vkResult);

        memcpy(data, position_256_graphics, bufferSize);

        vkUnmapMemory(vkDevice, vertexData_position_256x256_graphics.vkDeviceMemory);
    }

    else if (meshWidth == 512 && meshHeight == 512)
    {
        populateSineWaveArraysForCPU(512, 512, animationTime);

        vkResult = vkMapMemory(vkDevice, vertexData_position_512x512_graphics.vkDeviceMemory, 0, bufferSize, 0, &data);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For vertexData_position_512x512_graphics : %d\n", __func__, vkResult);

        memcpy(data, position_512_graphics, bufferSize);

        vkUnmapMemory(vkDevice, vertexData_position_512x512_graphics.vkDeviceMemory);
    }

    else if (meshWidth == 2048 && meshHeight == 2048)
    {
        populateSineWaveArraysForCPU(2048, 2048, animationTime);

        vkResult = vkMapMemory(vkDevice, vertexData_position_1024x1024_graphics.vkDeviceMemory, 0, bufferSize, 0, &data);
        if (vkResult != VK_SUCCESS)
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkMapMemory() Failed For vertexData_position_1024x1024_graphics : %d\n", __func__, vkResult);

        memcpy(data, position_1024_graphics, bufferSize);

        vkUnmapMemory(vkDevice, vertexData_position_1024x1024_graphics.vkDeviceMemory);
    }

    return vkResult;
  
}

void populateSineWaveArraysForCPU(uint32_t meshWidth, uint32_t meshHeight, float animationTime)
{
    for (unsigned int i = 0; i < meshWidth; i++)
    {
        for (unsigned int j = 0; j < meshHeight; j++)
        {
            for (unsigned int k = 0; k < 4; k++)
            {
                float u = (float)i / (float)meshWidth;
                float v = (float)j / (float)meshHeight;

                u = u * 2.0f - 1.0f;
                v = v * 2.0f - 1.0f;

                float frequency = 4.0f;

                float w = sinf(u * frequency + animationTime) * cosf(v * frequency + animationTime) * 0.5f;
                
                if (meshWidth == 64 && meshHeight == 64)
                {
                    if (k == 0)
                        position_64_graphics[i][j][k] = u;
                    if (k == 1)
                        position_64_graphics[i][j][k] = w;
                    if (k == 2)
                        position_64_graphics[i][j][k] = v;
                    if (k == 3)
                        position_64_graphics[i][j][k] = 1.0f;
                }
                else if (meshWidth == 128 && meshHeight == 128)
                {
                    if (k == 0)
                        position_128_graphics[i][j][k] = u;
                    if (k == 1)
                        position_128_graphics[i][j][k] = w;
                    if (k == 2)
                        position_128_graphics[i][j][k] = v;
                    if (k == 3)
                        position_128_graphics[i][j][k] = 1.0f;
                }
                else if (meshWidth == 256 && meshHeight == 256)
                {
                    if (k == 0)
                        position_256_graphics[i][j][k] = u;
                    if (k == 1)
                        position_256_graphics[i][j][k] = w;
                    if (k == 2)
                        position_256_graphics[i][j][k] = v;
                    if (k == 3)
                        position_256_graphics[i][j][k] = 1.0f;
                }
                else if (meshWidth == 512 && meshHeight == 512)
                {
                    if (k == 0)
                        position_512_graphics[i][j][k] = u;
                    if (k == 1)
                        position_512_graphics[i][j][k] = w;
                    if (k == 2)
                        position_512_graphics[i][j][k] = v;
                    if (k == 3)
                        position_512_graphics[i][j][k] = 1.0f;
                }
    
                else if (meshWidth == 1024 && meshHeight == 1024)
                {
                    if (k == 0)
                        position_1024_graphics[i][j][k] = u;
                    if (k == 1)
                        position_1024_graphics[i][j][k] = w;
                    if (k == 2)
                        position_1024_graphics[i][j][k] = v;
                    if (k == 3)
                        position_1024_graphics[i][j][k] = 1.0f;
                }
    
                else if (meshWidth == 2048 && meshHeight == 2048)
                {
                    if (k == 0)
                        position_1024_graphics[i][j][k] = u;
                    if (k == 1)
                        position_1024_graphics[i][j][k] = w;
                    if (k == 2)
                        position_1024_graphics[i][j][k] = v;
                    if (k == 3)
                        position_1024_graphics[i][j][k] = 1.0f;
                }
               
            }
        }
    }
}


VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
    VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
    VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData
)
{
    // Code
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "ADN_VALIDATION : debugReportCallback() => %s(%d) = %s\n", pLayerPrefix, messageCode, pMessage);
    return VK_FALSE;
}


//! Compute Shader Related
VkResult initialize_compute(void)
{
    // Function Declarations
    VkResult createCommandBuffer_compute(void);
    VkResult createStorageBuffer_compute(void);
    VkResult createShader_compute(void);
    VkResult createDescriptorSetLayout_compute(void);
    VkResult createPipelineLayout_compute(void);
    VkResult createDescriptorSet_compute(void);
    VkResult createPipeline_compute(void);
    VkResult buildCommandBuffer_compute(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = createCommandBuffer_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffer_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createCommandBuffer_compute() Succeeded\n", __func__);

    vkResult = createStorageBuffer_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createStorageBuffer_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createStorageBuffer_compute() Succeeded\n", __func__);

    vkResult = createShader_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createShader_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createShader_compute() Succeeded\n", __func__);

    vkResult = createDescriptorSetLayout_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSetLayout_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSetLayout_compute() Succeeded\n", __func__);

    vkResult = createPipelineLayout_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipelineLayout_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipelineLayout_compute() Succeeded\n", __func__);

    vkResult = createDescriptorSet_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSet_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createDescriptorSet_compute() Succeeded\n", __func__);

    vkResult = createPipeline_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipeline_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => createPipeline_compute() Succeeded\n", __func__);

    vkResult = buildCommandBuffer_compute();
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffer_compute() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => buildCommandBuffer_compute() Succeeded\n", __func__);

    return vkResult;
}

VkResult createCommandBuffer_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; 
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_compute);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateCommandBuffers() Failed For vkCommandBuffer_compute : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateCommandBuffers() Succeeded For vkCommandBuffer_compute\n", __func__);

    return vkResult;
}

VkResult createStorageBuffer_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    VkDeviceSize bufferSize = 4096 * 4096 * 4 * sizeof(float);

    memset((void*)&computeBuffer, 0, sizeof(ComputeData));

    //* Step - 5
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.flags = 0;   //! Valid Flags are used in sparse(scattered) buffers
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.size = bufferSize;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //!                                 COMPUTE WRITE                          GRAPHICS READ
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    //* Step - 6
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &computeBuffer.vkBuffer);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Failed For Compute Storage Buffer  : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateBuffer() Succeeded For Compute Storage Buffer\n", __func__);
    
    //* Step - 7
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, computeBuffer.vkBuffer, &vkMemoryRequirements);

    //* Step - 8
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
    
    //* Step - 8.1
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        //* Step - 8.2
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            //* Step - 8.3
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                //* Step - 8.4
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        //* Step - 8.5
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //* Step - 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &computeBuffer.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Failed For Compute Storage Buffer : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateMemory() Succeeded For Compute Storage Buffer\n", __func__);

    //* Step - 10
    //! Binds Vulkan Device Memory Object Handle with the Vulkan Buffer Object Handle
    vkResult = vkBindBufferMemory(vkDevice, computeBuffer.vkBuffer, computeBuffer.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Failed For Compute Storage Buffer : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBindBufferMemory() Succeeded For Compute Storage Buffer\n", __func__);
    

    return vkResult;
}

VkResult createShader_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    AAsset* shaderAsset = NULL;

    const char* szFileName = "Shader.comp.spv";
    shaderAsset = AAssetManager_open(androidAssetManager, szFileName, AASSET_MODE_BUFFER);
    if (shaderAsset == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Open SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Succeeded In Opening SPIR-V Shader File : %s\n", __func__, szFileName);

    size_t size = AAsset_getLength(shaderAsset);

    char* shaderData = (char*)malloc(size * sizeof(char));
    if (shaderData == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For shaderData !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    size_t retVal = AAsset_read(shaderAsset, shaderData, size);
    if (retVal < 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Read From SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Successfully Read Shader From SPIR-V Shader File : %s\n", __func__, szFileName);
    
    if (shaderAsset)
    {
        AAsset_close(shaderAsset); 
        shaderAsset = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Closed SPIR-V File : %s\n", __func__, szFileName);
    }

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //! Reserved, must be 0
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;
    vkShaderModuleCreateInfo.codeSize = size;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_compute_shader);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Failed For Compute Shader : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateShaderModule() Succeeded For Compute Shader\n", __func__);

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => free() Succeeded For shaderData\n", __func__);
    }

    __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Compute Shader Module Successfully Created\n", __func__);

    return vkResult;
}

VkResult createDescriptorSetLayout_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //! Initialize VkDescriptorSetLayoutBinding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
    memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vkDescriptorSetLayoutBinding.binding = 1;
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;

    //* Step - 3
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;   //! An integer value where you want to bind descriptor set
    vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;

    //* Step - 4
    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_compute);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorSetLayout() Failed For Compute : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateDescriptorSetLayout() Succeeded For Compute\n", __func__);

    return vkResult;
}

VkResult createPipelineLayout_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    VkPushConstantRange vkPushConstantRange;
    memset((void*)&vkPushConstantRange, 0, sizeof(VkPushConstantRange));
    vkPushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    vkPushConstantRange.offset = 0;
    vkPushConstantRange.size = sizeof(PushConstants);

    //* Step - 3
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout_compute;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange;

    //* Step - 4
    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout_compute);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineLayout() Failed For Compute : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineLayout() Succeeded For Compute\n", __func__);

    return vkResult;
}

VkResult createDescriptorSet_compute(void)
{
    // Variable Declarations
    VkResult vkResult;

    // Code

    //* Initialize DescriptorSetAllocationInfo
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout_compute;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_compute);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateDescriptorSets() Failed For vkDescriptorSet_compute : %d !!!\n", __func__, vkResult);
        return vkResult;
    }  
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkAllocateDescriptorSets() Succeeded For vkDescriptorSet_compute\n", __func__);
    
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = computeBuffer.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = VK_WHOLE_SIZE;

    /* Update above descriptor set directly to the shader
    There are 2 ways :-
        1) Writing directly to the shader
        2) Copying from one shader to another shader
    */
    VkWriteDescriptorSet vkWriteDescriptorSet;
    memset((void*)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));
    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet_compute;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 1;

    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

    return vkResult;
}

VkResult createPipeline_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Code

    //! Shader Stage State
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo;
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    
    //* Vertex Shader
    vkPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo.pNext = NULL;
    vkPipelineShaderStageCreateInfo.flags = 0;
    vkPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    vkPipelineShaderStageCreateInfo.module = vkShaderModule_compute_shader;
    vkPipelineShaderStageCreateInfo.pName = "main";
    vkPipelineShaderStageCreateInfo.pSpecializationInfo = NULL;

    VkComputePipelineCreateInfo vkComputePipelineCreateInfo;
    memset((void*)&vkComputePipelineCreateInfo, 0, sizeof(VkComputePipelineCreateInfo));
    vkComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    vkComputePipelineCreateInfo.pNext = NULL;
    vkComputePipelineCreateInfo.flags = 0;
    vkComputePipelineCreateInfo.stage = vkPipelineShaderStageCreateInfo;
    vkComputePipelineCreateInfo.layout = vkPipelineLayout_compute;

    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    
    VkPipelineCache vkPipelineCache_compute = VK_NULL_HANDLE;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache_compute);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineCache() Failed For Compute : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreatePipelineCache() Succeeded For Compute\n", __func__);
    
    vkResult = vkCreateComputePipelines(vkDevice, vkPipelineCache_compute, 1, &vkComputePipelineCreateInfo, NULL, &vkPipeline_compute);
    if (vkResult != VK_SUCCESS)
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateComputePipelines() Failed : %d !!!\n", __func__, vkResult);
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkCreateComputePipelines() Succeeded\n", __func__);

    //* Destroy Pipeline Cache
    if (vkPipelineCache_compute)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache_compute, NULL);
        vkPipelineCache_compute = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkDestroyPipelineCache() Succeeded\n", __func__);
    }
   

    return vkResult;
}

VkResult buildCommandBuffer_compute(void)
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = vkResetCommandBuffer(vkCommandBuffer_compute, 0);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkResetCommandBuffer() Failed : %d\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
    memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo.pNext = NULL;
    vkCommandBufferBeginInfo.flags = 0;     //! 0 specifies that we will use only the primary command buffer, and not going to use this command buffer simultaneously between multiple threads

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_compute, &vkCommandBufferBeginInfo);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkBeginCommandBuffer() Failed For vkCommandBuffer_compute : %d\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    vkCmdBindPipeline(vkCommandBuffer_compute, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline_compute);

    vkCmdBindDescriptorSets(
        vkCommandBuffer_compute,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        vkPipelineLayout_compute,
        0,
        1,
        &vkDescriptorSet_compute,
        0,
        0
    );

    PushConstants pushData;
    memset((void*)&pushData, 0, sizeof(PushConstants));
    pushData.width = 2048;
    pushData.height = 2048;
    pushData.time = fAnimationSpeed;

    vkCmdPushConstants(
        vkCommandBuffer_compute,
        vkPipelineLayout_compute,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(PushConstants),
        &pushData
    );

    vkCmdDispatch(vkCommandBuffer_compute, (2048 + 15) / 16, (2048 + 15) / 16, 1);

    vkResult = vkEndCommandBuffer(vkCommandBuffer_compute);
    if (vkResult != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => vkEndCommandBuffer() Failed For vkCommandBuffer_compute : %d\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    return vkResult;
}

void uninitialize_compute(void)
{
    // Code
    if (vkPipeline_compute)
    {
        vkDestroyPipeline(vkDevice, vkPipeline_compute, NULL);
        vkPipeline_compute = VK_NULL_HANDLE;
    }

    if (vkPipelineLayout_compute)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout_compute, NULL);
        vkPipelineLayout_compute = VK_NULL_HANDLE;
    }

    if (vkDescriptorSetLayout_compute)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout_compute, NULL);
        vkDescriptorSetLayout_compute = VK_NULL_HANDLE;
    }
    
    if (vkShaderModule_compute_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_compute_shader, NULL);
        vkShaderModule_compute_shader = VK_NULL_HANDLE;
    }

    if (computeBuffer.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, computeBuffer.vkDeviceMemory, NULL);
        computeBuffer.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if (computeBuffer.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, computeBuffer.vkBuffer, NULL);
        computeBuffer.vkBuffer = VK_NULL_HANDLE;
    }

    if (vkCommandBuffer_compute)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_compute);
        vkCommandBuffer_compute = NULL;
    }

}

