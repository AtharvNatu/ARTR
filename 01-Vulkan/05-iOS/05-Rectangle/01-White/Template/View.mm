#import "View.h"

//! Vulkan Related MoltenVk Header
#include <MoltenVK/mvk_vulkan.h>

//! GLM Related Macros and Header Files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Macros
#define _ARRAYSIZE(x)       (sizeof(x) / sizeof((x)[0]))

// Global Variable Declarations
int winWidth = 0;
int winHeight = 0;
const char *gpSzAppName = "ARTR";

//! Vulkan Related Global Variables
uint32_t enabledInstanceExtensionCount = 0;

//* VK_KHR_SURFACE_EXTENSION_NAME,
//* VK_EXT_METAL_SURFACE_EXTENSION_NAME,
//* VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
//* VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
//* VK_EXT_DEBUG_UTILS_EXTENSION_NAME
const char *enabledInstanceExtensionNames_array[5];
VkBool32 vulkanPortabilityEnumerationExtensionFound = VK_FALSE;

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

//* VK_KHR_SWAPCHAIN_EXTENSTION_NAME,
//* VK_KHR_PORTABILITY_SUBSET_EXTENSTION_NAME
const char *enabledDeviceExtensionNames_array[2];

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

//? Swapchain Images and Image Views -> For Color Images
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

//? Command Pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//? Command Buffer
VkCommandBuffer *vkCommandBuffer_array = NULL;

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

//? Render
BOOL bInitialized = NO;
uint32_t currentImageIndex = UINT32_MAX;

//? Validation
BOOL bValidation = YES;
uint32_t enabledValidationLayerCount = 0;
const char *enabledValidationLayerNames_array[1];   //* For VK_LAYER_KHRONOS_validation
VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT_fnptr = NULL;

//? Vertex Buffer Related Variables
typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

//? Position Related Variables
VertexData vertexData_position;

//? Uniform Related Variables
typedef struct
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
} MVP_UniformData;

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


@implementation View
{
    @private
    CADisplayLink* displayLink;
    BOOL bDisplayLinkActive;
}

-(id) initWithFrame:(CGRect)frameRect
{
    // Variable Declarations
    UITapGestureRecognizer* singleTapGestureRecognizer = nil;
    UITapGestureRecognizer* doubleTapGestureRecognizer = nil;
    UISwipeGestureRecognizer* swipeGestureRecognizer = nil;
    UILongPressGestureRecognizer* longPressGestureRecognizer = nil;
    
    // Code
    self = [super initWithFrame:frameRect];
    if (self)
    {
        winWidth = [self bounds].size.width;
        winHeight = [self bounds].size.height;
        
        // Initialize
        VkResult vkResult = [self initialize];
           if (vkResult != VK_SUCCESS)
               printf("\n%s() => initialize() Failed : %d !!!\n", __func__, vkResult);
           else
               printf("\n%s() => initialize() Succeeded\n", __func__);
        
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
        
        bDisplayLinkActive = NO;
        
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

-(void) startDisplayLink
{
    // Initialize Display Link
    if (!bDisplayLinkActive)
    {
        NSUInteger animationFrameInterval = 60;
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        bDisplayLinkActive = YES;
    }
    
}

-(void) stopDisplayLink
{
    if (bDisplayLinkActive)
    {
        [displayLink invalidate];
        displayLink = nil;
        bDisplayLinkActive = NO;
    }
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

-(VkResult) initialize
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = [self createVulkanInstance];
    if (vkResult != VK_SUCCESS)
        printf("%s() => createVulkanInstance() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => createVulkanInstance() Succeeded\n", __func__);

    //! Create Vulkan Presentation Surface
    vkResult = [self getSupportedSurface];
    if (vkResult != VK_SUCCESS)
        printf("%s() => getSupportedSurface() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => getSupportedSurface() Succeeded\n", __func__);

    //! Enumerate and Select Required Physical Device and its Queue Family Index
    vkResult = [self getPhysicalDevice];
    if (vkResult != VK_SUCCESS)
        printf("%s() => getPhysicalDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => getPhysicalDevice() Succeeded\n", __func__);

    //! Print Vulkan Info
    vkResult = [self printVkInfo];
    if (vkResult != VK_SUCCESS)
        printf("%s() => printVkInfo() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => printVkInfo() Succeeded\n", __func__);

    //! Create Vulkan Device
    vkResult = [self createVulkanDevice];
    if (vkResult != VK_SUCCESS)
        printf("%s() => createVulkanDevice() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => createVulkanDevice() Succeeded\n", __func__);

    //! Get Device Queue
    [self getDeviceQueue];

    //! Create Swapchain
    vkResult = [self createSwapchain:VK_FALSE];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createSwapchain() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createSwapchain() Succeeded\n", __func__);

    //! Create Swapchain Image and Image Views
    vkResult = [self createImagesAndImageViews];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createImagesAndImageViews() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createImagesAndImageViews() Succeeded\n", __func__);
    
    //! Create Command Pool
    vkResult = [self createCommandPool];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createCommandPool() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createCommandPool() Succeeded\n", __func__);

    //! Create Command Buffers
    vkResult = [self createCommandBuffers];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createCommandBuffers() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createCommandBuffers() Succeeded\n", __func__);

    //! Create Vertex Buffer
    vkResult = [self createVertexBuffer];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createVertexBuffer() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createVertexBuffer() Succeeded\n", __func__);

    //! Create Uniform Buffer
    vkResult = [self createUniformBuffer];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createUniformBuffer() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createUniformBuffer() Succeeded\n", __func__);

    //! Create Shaders
    vkResult = [self createShaders];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createShaders() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createShaders() Succeeded\n", __func__);

    //! Create Descriptor Set Layout
    vkResult = [self createDescriptorSetLayout];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createDescriptorSetLayout() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createDescriptorSetLayout() Succeeded\n", __func__);

    //! Create Pipeline Layout
    vkResult = [self createPipelineLayout];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createPipelineLayout() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createPipelineLayout() Succeeded\n", __func__);

    //! Create Descriptor Pool
    vkResult = [self createDescriptorPool];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createDescriptorPool() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createDescriptorPool() Succeeded\n", __func__);

    //! Create Descriptor Set
    vkResult = [self createDescriptorSet];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createDescriptorSet() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createDescriptorSet() Succeeded\n", __func__);

    //! Create Render Pass
    vkResult = [self createRenderPass];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createRenderPass() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createRenderPass() Succeeded\n", __func__);

    //! Create Pipeline
    vkResult = [self createPipeline];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createPipeline() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createPipeline() Succeeded\n", __func__);

    //! Create Framebuffers
    vkResult = [self createFramebuffers];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createFramebuffers() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createFramebuffers() Succeeded\n", __func__);

    //! Create Semaphores
    vkResult = [self createSemaphores];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createSemaphores() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createSemaphores() Succeeded\n", __func__);

    //! Create Fences
    vkResult = [self createFences];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => createFences() Failed : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => createFences() Succeeded\n", __func__);

    //! Initialize Clear Color Values (Analogous to glClearColor())
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;    //* R
    vkClearColorValue.float32[1] = 0.0f;    //* G
    vkClearColorValue.float32[2] = 1.0f;    //* B
    vkClearColorValue.float32[3] = 1.0f;    //* A

    vkResult = [self buildCommandBuffers];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => buildCommandBuffers() Failed\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => buildCommandBuffers() Succeeded\n", __func__);

    //! Initialization Completed
    bInitialized = YES;
    printf("%s() => Initialization Completed Successfully\n", __func__);
    
    return vkResult;
}

-(VkResult) resize:(int)width :(int)height
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (height <= 0)
        height = 1;

    //* Check the bInitialized Variable
    if (bInitialized == NO)
    {
        printf("%s() => Initialization Not Yet Completed or Failed !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    //* As recreation of swapchain is needed, we are going to repeat many steps of initialize() again. Hence, set bInitialize = NO again
    bInitialized = NO;
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
            printf("%s() => Swapchain is already NULL ... cannot proceed !!!\n", __func__);
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
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        if (vkCommandBuffer_array)
        {
            free(vkCommandBuffer_array);
            vkCommandBuffer_array = NULL;
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
        //     printf("%s() => vkDestroyImage() Succeeded\n", __func__);
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
        vkResult = [self createSwapchain:VK_FALSE];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createSwapchain() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Swapchain Image and Image Views
        vkResult = [self createImagesAndImageViews];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createImagesAndImageViews() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Render Pass
        vkResult = [self createRenderPass];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createRenderPass() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Pipeline Layout
        vkResult = [self createPipelineLayout];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createPipelineLayout() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Pipeline
        vkResult = [self createPipeline];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createPipeline() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Command Buffers
        vkResult = [self createCommandBuffers];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createCommandBuffers() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Create Framebuffers
        vkResult = [self createFramebuffers];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createFramebuffers() Failed : %d !!!\n", __func__, vkResult);
            return vkResult;
        }

        //* Build Command Buffers
        vkResult = [self buildCommandBuffers];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => buildCommandBuffers() Failed\n", __func__);
            return vkResult;
        }
        //?--------------------------------------------------------------------------------------------------
    }
    bInitialized = YES;

    return vkResult;
}

-(VkResult) render
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (bInitialized == NO)
    {
        printf("%s() => Initialization Not Yet Completed !!!\n", __func__);
        return (VkResult)VK_FALSE;
    }

    //! For macOS - VK_ERROR_OUT_OF_DATE_KHR and VK_SUBOPTIMAL_KHR are normal, and for these conditions, resize() should not be called
    //! Resize Functionality is automatically handled by NSView through delegate windowWillResize, through which call to resize() has been done

    //! Acquire next image index
    vkResult = vkAcquireNextImageKHR(vkDevice, vkSwapchainKHR, UINT64_MAX, vkSemaphore_backBuffer, VK_NULL_HANDLE, &currentImageIndex);
    if (vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
    {
        printf("%s() => vkAcquireNextImageKHR() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    //! Use fence to allow host to wait for completion of execution of previous command buffer
    vkResult = vkWaitForFences(vkDevice, 1, &vkFence_array[currentImageIndex], VK_TRUE, UINT64_MAX);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkWaitForFences() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    //! Make sure fences are ready for execution of next command buffer
    vkResult = vkResetFences(vkDevice, 1, &vkFence_array[currentImageIndex]);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkResetFences() Failed : %d\n", __func__, vkResult);
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
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;

    //! Submit above work to the queue
    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_array[currentImageIndex]);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkQueueSubmit() Failed : %d\n", __func__, vkResult);
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
    if (vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
    {
        printf("%s() => vkQueuePresentKHR() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    vkDeviceWaitIdle(vkDevice);

    vkResult = [self updateUniformBuffer];
    if (vkResult != VK_SUCCESS)
        printf("%s() => updateUniformBuffer() Failed : %d\n", __func__, vkResult);

    vkDeviceWaitIdle(vkDevice);

    return vkResult;
}

-(void) update
{
    // Code
}

-(void) uninitialize
{
    // Code
    
    //* Step - 5 of Device Creation (Destroy Vulkan Device)
    //! vkDeviceWaitIdle(vkDevice) should be the 1st API to maintain synchronization
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        printf("%s() => vkDeviceWaitIdle() Succeeded\n", __func__);
    }

    //* Step - 7 of Fences and Semaphores
    if (vkFence_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFence(vkDevice, vkFence_array[i], NULL);
            printf("%s() => vkDestroyFence() Succeeded For Index : %d\n", __func__, i);
        }

        free(vkFence_array);
        vkFence_array = NULL;
        printf("%s() => free() Succeeded For vkFence_array\n", __func__);
    }

    if (vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
        printf("%s() => vkDestroySemaphore() Succeeded For vkSemaphore_renderComplete\n", __func__);
        vkSemaphore_renderComplete = VK_NULL_HANDLE;
    }

    if (vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
        printf("%s() => vkDestroySemaphore() Succeeded For vkSemaphore_backBuffer\n", __func__);
        vkSemaphore_backBuffer = VK_NULL_HANDLE;
    }

    //* Step - 5 of Frame Buffer
    if (vkFramebuffer_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
            printf("%s() => vkDestroyFramebuffer() Succeeded For Index : %d\n", __func__, i);
        }

        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        printf("%s() => free() Succeeded For vkFramebuffer_array\n", __func__);
    }

    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        printf("%s() => vkDestroyPipeline() Succeeded\n", __func__);
    }

    //* Step - 6 of Render Pass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        printf("%s() => vkDestroyRenderPass() Succeeded\n", __func__);
    }

    //* Destroy Descriptor Pool (Destroys Descriptor Set with it)
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        vkDescriptorSet = VK_NULL_HANDLE;
        printf("%s() => vkDestroyDescriptorPool() => Destroyed vkDescriptorPool and vkDescriptorSet Successfully\n", __func__);
    }

    //* Step - 5 of PipelineLayout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        printf("%s() => vkDestroyPipelineLayout() Succeeded\n", __func__);
    }

    //* Step - 5 of DescriptorSetLayout
    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        printf("%s() => vkDestroyDescriptorSetLayout() Succeeded\n", __func__);
    }

    //* Step - 11 of Shaders
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        printf("%s() => vkDestroyShaderModule() Succeeded For Fragment Shader\n", __func__);
    }

    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        printf("%s() => vkDestroyShaderModule() Succeeded For Vertex Shader\n", __func__);
    }

    //* Destroy Uniform Buffer
    if (uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        printf("%s() => vkFreeMemory() Succeeded For uniformData.vkDeviceMemory\n", __func__);
    }

    if (uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        printf("%s() => vkDestroyBuffer() Succeeded For uniformData.vkBuffer\n", __func__);
    }

    //* Step - 14 of Vertex Buffer
    if (vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
        vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        printf("%s() => vkFreeMemory() Succeeded For vertexData_position.vkDeviceMemory\n", __func__);
    }

    if (vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
        vertexData_position.vkBuffer = VK_NULL_HANDLE;
        printf("%s() => vkDestroyBuffer() Succeeded For vertexData_position.vkBuffer\n", __func__);
    }

    //* Step - 5 of Command Buffer
    if (vkCommandBuffer_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
            printf("%s() => vkFreeCommandBuffers() Succeeded For Index : %d\n", __func__, i);
        }

        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        printf("%s() => free() Succeeded For vkCommandBuffer_array\n", __func__);
    }

    //* Step - 4 of Command Pool (Destroy Command Pool)
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        printf("%s() => vkDestroyCommandPool() Succeeded\n", __func__);
    }

    //* Step - 7, 8 of Swapchain Image and Image Views
    if (swapchainImageView_array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
            vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        printf("%s() => vkDestroyImageView() Succeeded\n", __func__);

        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        printf("%s() => free() Succeeded For swapchainImageView_array\n", __func__);
    }

    //! No need to free swapchain images ->  Uncommenting causes the code to crash
    // for (uint32_t i = 0; i < swapchainImageCount; i++)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
    //     printf("%s() => vkDestroyImage() Succeeded\n", __func__);
    // }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        printf("%s() => free() Succeeded For swapchainImage_array\n", __func__);
    }

    //* Step - 10 of Swapchain (Destroy Swapchain)
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        printf("%s() => vkDestroySwapchainKHR() Succeeded\n", __func__);
    }

    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        printf("%s() => vkDestroyDevice() Succeeded\n", __func__);
    }

    //* No need to destroy device queue

    //* No need to destroy selected physical device

    //* Step - 5 of Presentation Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
        printf("%s() => vkDestroySurfaceKHR() Succeeded\n", __func__);
    }

    if (vkDebugUtilsMessengerEXT && vkDestroyDebugUtilsMessengerEXT_fnptr)
    {
        vkDestroyDebugUtilsMessengerEXT_fnptr(vkInstance, vkDebugUtilsMessengerEXT, NULL);
        vkDebugUtilsMessengerEXT = VK_NULL_HANDLE;
        vkDestroyDebugUtilsMessengerEXT_fnptr = NULL;
    }

    //* Step - 5 of Instance Creation
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        printf("%s() => vkDestroyInstance() Succeeded\n", __func__);
    }
}

//! Definition of Vulkan Related Functions
-(VkResult) createVulkanInstance
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = [self fillInstanceExtensionNames];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => fillInstanceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
        printf("%s() => fillInstanceExtensionNames() Succeeded\n", __func__);

    //! Fill Validation Layers
    if (bValidation == YES)
    {
        vkResult = [self fillValidationLayerNames];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => fillValidationLayerNames() Failed : %d !!!\n", __func__, vkResult);
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        else
            printf("%s() => fillValidationLayerNames() Succeeded\n", __func__);
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
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

    //* Step - 3
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;

    //! The following flag must be specified when using the Khronos Vulkan ICD Loader instead of linking directly against the MoltenVK Framework
    //! Condition checking is mandatory on iOS
    if (vulkanPortabilityEnumerationExtensionFound)
    {
        vkInstanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
    else
        vkInstanceCreateInfo.flags = 0;
    

    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    if (bValidation == YES)
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
        printf("%s() => vkCreateInstance() Failed Due To Incompatible Driver : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        printf("%s() => vkCreateInstance() Failed Because Required Extension Is Not Present : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkCreateInstance() Failed : %d!!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkCreateInstance() Succeeded\n", __func__);

    //! Handling Validation Callbacks
    if (bValidation == YES)
    {
        vkResult = [self createValidationCallbackFunction];
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => createValidationCallbackFunction() Failed : %d !!!\n", __func__, vkResult);
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        else
            printf("%s() => createValidationCallbackFunction() Succeeded\n", __func__);
    }

    return vkResult;
}

-(VkResult) fillInstanceExtensionNames
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t instanceExtensionCount = 0;
    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 1 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(instanceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        printf("%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 2 : vkEnumerateInstanceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **instanceExtensionNames_array = NULL;
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    if (instanceExtensionNames_array == NULL)
    {
        printf("%s() => malloc() Failed For instanceExtensionNames_array !!!\n", __func__);
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
            printf("%s() => malloc() Failed For instanceExtensionNames_array[%d] !!!\n", __func__, i);
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

        printf("%s() => Vulkan Instance Extension Name : %s\n", __func__, instanceExtensionNames_array[i]);
    }

    //* Step - 4
    if (vkExtensionProperties_array)
    {
        free(vkExtensionProperties_array);
        vkExtensionProperties_array = NULL;
    }

    //* Step - 5
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 metalSurfaceExtensionFound = VK_FALSE;
    VkBool32 getPhysicalDeviceProperties2ExtensionFound = VK_FALSE;
    VkBool32 debugUtilsExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
           
        if (strcmp(instanceExtensionNames_array[i], VK_EXT_METAL_SURFACE_EXTENSION_NAME) == 0)
        {
            metalSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
        }
           
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
        {
            vulkanPortabilityEnumerationExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
        }
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
        {
            getPhysicalDeviceProperties2ExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
        }

        if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            debugUtilsExtensionFound = VK_TRUE;
            if (bValidation == YES)
                enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            else
            {
                // Array will not have entry of VK_EXT_DEBUG_UTILS_EXTENSION_NAME
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
        printf("%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_KHR_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    if (metalSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => VK_EXT_METAL_SURFACE_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_EXT_METAL_SURFACE_EXTENSION_NAME Extension Found\n", __func__);

    if (vulkanPortabilityEnumerationExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME Extension Found\n", __func__);
    
    if (getPhysicalDeviceProperties2ExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME Extension Found\n", __func__);

    if (debugUtilsExtensionFound == VK_FALSE)
    {
        if (bValidation == YES)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            printf("%s() => VALIDATION ON : VK_EXT_DEBUG_UTILS_EXTENSION_NAME Extension Not Supported !!!\n", __func__);
            return vkResult;
        }
        else
            printf("%s() => VALIDATION OFF : VK_EXT_DEBUG_UTILS_EXTENSION_NAME Extension Not Supported !!!\n", __func__);
    }
    else
    {
        if (bValidation == YES)
            printf("%s() => VALIDATION ON : VK_EXT_DEBUG_UTILS_EXTENSION_NAME Extension Supported\n", __func__);
        else
            printf("%s() => VALIDATION OFF : VK_EXT_DEBUG_UTILS_EXTENSION_NAME Extension Supported\n", __func__);
    }

    //* Step - 8
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
        printf("%s() => Enabled Vulkan Instance Extension Name : %s\n", __func__, enabledInstanceExtensionNames_array[i]);

    return vkResult;
}

-(VkResult) fillValidationLayerNames
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    uint32_t validationLayerCount = 0;
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => Call 1 : vkEnumerateInstanceLayerProperties() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => Call 1 : vkEnumerateInstanceLayerProperties() Succeeded\n", __func__);

    VkLayerProperties *vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*)malloc(validationLayerCount * sizeof(VkLayerProperties));
    if (vkLayerProperties_array == NULL)
    {
        printf("%s() => malloc() Failed For vkLayerProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
    if (vkResult != VK_SUCCESS)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => Call 2 : vkEnumerateInstanceLayerProperties() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => Call 2 : vkEnumerateInstanceLayerProperties() Succeeded\n", __func__);

    char **validationLayerNames_array = NULL;
    validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
    if (validationLayerNames_array == NULL)
    {
        printf("%s() => malloc() Failed For validationLayerNames_array !!!\n", __func__);
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
            printf("%s() => malloc() Failed For validationLayerNames_array[%d] !!!\n", __func__, i);
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

        printf("%s() => Vulkan Instance Layer Name : %s\n", __func__, validationLayerNames_array[i]);
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
        printf("%s() => VK_LAYER_KHRONOS_validation Not Supported !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_LAYER_KHRONOS_validation Supported\n", __func__);

    for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
        printf("%s() => Enabled Vulkan Validation Layer Name : %s\n", __func__, enabledValidationLayerNames_array[i]);

    return vkResult;
}

-(VkResult) createValidationCallbackFunction
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_fnptr = NULL;
    
    // Code
    
    //! Get the required function pointers
    vkCreateDebugUtilsMessengerEXT_fnptr = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (vkCreateDebugUtilsMessengerEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
                printf("%s() => vkGetInstanceProcAddr() Failed To Get Function Pointer For vkCreateDebugUtilsMessengerEXT !!!\n", __func__);
                return vkResult;
    }
    else
            printf("%s() => vkGetInstanceProcAddr() Succeeded To Get Function Pointer For vkCreateDebugUtilsMessengerEXT\n", __func__);
    
    vkDestroyDebugUtilsMessengerEXT_fnptr = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (vkDestroyDebugUtilsMessengerEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
                printf("%s() => vkGetInstanceProcAddr() Failed To Get Function Pointer For vkDestroyDebugUtilsMessengerEXT !!!\n", __func__);
                return vkResult;
    }
    else
            printf("%s() => vkGetInstanceProcAddr() Succeeded To Get Function Pointer For vkDestroyDebugUtilsMessengerEXT\n", __func__);
    
    //* Get the Vulkan Debug Utils Callback Object
    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT;
    memset((void*)&vkDebugUtilsMessengerCreateInfoEXT, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    vkDebugUtilsMessengerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vkDebugUtilsMessengerCreateInfoEXT.pNext = NULL;
    vkDebugUtilsMessengerCreateInfoEXT.flags = 0;
    vkDebugUtilsMessengerCreateInfoEXT.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    vkDebugUtilsMessengerCreateInfoEXT.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vkDebugUtilsMessengerCreateInfoEXT.pfnUserCallback = debugUtilsMessengerCallback;
    vkDebugUtilsMessengerCreateInfoEXT.pUserData = NULL;

    vkResult = vkCreateDebugUtilsMessengerEXT_fnptr(vkInstance, &vkDebugUtilsMessengerCreateInfoEXT, NULL, &vkDebugUtilsMessengerEXT);
    if (vkResult != VK_SUCCESS)
    {
       printf("%s() => vkCreateDebugUtilsMessengerEXT_fnptr() Failed : %d !!!\n", __func__, vkResult);
       return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
       printf("%s() => vkCreateDebugUtilsMessengerEXT_fnptr() Succeeded\n", __func__);
    
    
    return vkResult;
}

-(VkResult) getSupportedSurface
{
    // Code

    //* Step - 1
    VkMetalSurfaceCreateInfoEXT vkMetalSurfaceCreateInfoEXT;
    VkResult vkResult = VK_SUCCESS;

    //* Step - 2
    memset((void*)&vkMetalSurfaceCreateInfoEXT, 0, sizeof(VkMetalSurfaceCreateInfoEXT));

    //* Step - 3
    vkMetalSurfaceCreateInfoEXT.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    vkMetalSurfaceCreateInfoEXT.pNext = NULL;
    vkMetalSurfaceCreateInfoEXT.flags = 0;
    vkMetalSurfaceCreateInfoEXT.pLayer = (CAMetalLayer*)[self layer];

    //* Step - 4
    
    //! Use a function pointer to invoke vkCreateMetalSurfaceEXT for compatibility across all iOS devices
    PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT_fnptr = NULL;
    
    vkCreateMetalSurfaceEXT_fnptr = (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateMetalSurfaceEXT");
    if (vkCreateMetalSurfaceEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => vkGetInstanceProcAddr() Failed To Get Function Pointer For vkCreateMetalSurfaceEXT : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    
    vkResult = vkCreateMetalSurfaceEXT_fnptr(vkInstance, &vkMetalSurfaceCreateInfoEXT, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkCreateMetalSurfaceEXT_fnptr() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateMetalSurfaceEXT_fnptr() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) getPhysicalDevice
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    
    // Code

    //* Step - 2
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (vkResult == VK_SUCCESS)
        printf("%s() Call 1 => vkEnumeratePhysicalDevices() Succeeded\n", __func__);
    else if (physicalDeviceCount == 0)
    {
        printf("%s() => vkEnumeratePhysicalDevices() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        printf("%s() Call 1 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //* Step - 3
    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (vkPhysicalDevice_array == NULL)
    {
        printf("%s() => malloc() Failed For vkPhysicalDevice_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 4
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() Call 2 => vkEnumeratePhysicalDevices() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
       printf("%s() Call 2 => vkEnumeratePhysicalDevices() Succeeded\n", __func__);

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
            printf("%s() => malloc() Failed For vkQueueFamilyProperties_array !!!\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        //* Step - 5.4
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        //* Step - 5.5
        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
        if (isQueueSurfaceSupported_array == NULL)
        {
            printf("%s() => malloc() Failed For isQueueSurfaceSupported_array\n", __func__);
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
            printf("%s() => free() Succeeded For isQueueSurfaceSupported_array\n", __func__);
            isQueueSurfaceSupported_array = NULL;
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            printf("%s() => free() Succeeded For vkQueueFamilyProperties_array\n", __func__);
            vkQueueFamilyProperties_array = NULL;
        }

        //* Step - 5.9
        if (bFound == VK_TRUE)
            break;
        
    }

    //* Step - 5.10
    if (bFound == VK_TRUE)
        printf("%s() => Succeeded To Obtain Graphics Supported Physical Device\n", __func__);

    //* Step - 6
    else
    {
        printf("%s() => Failed To Obtain Graphics Supported Physical Device !!!\n", __func__);
        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            printf("%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
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
        printf("%s() => Selected Physical Device Supports Tessellation Shader\n", __func__);
    else
        printf("%s() => Selected Physical Device Does Not Support Tessellation Shader !!!\n", __func__);

    if (vkPhysicalDeviceFeatures.geometryShader == VK_TRUE)
        printf("%s() => Selected Physical Device Supports Geometry Shader\n", __func__);
    else
        printf("%s() => Selected Physical Device Does Not Support Geometry Shader !!!\n", __func__);

    return vkResult;
}

-(VkResult) printVkInfo
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    printf("\nVULKAN INFORMATION\n");
    printf("------------------------------------------------------------------------------------------------");
    
    //* Step - 3.1
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        printf("\nDevice Number : %d\n", i);
        printf("*******************************************************\n");
        
        //* Step - 3.2
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

        //* Step - 3.3
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
        printf("Vulkan API Version : %u.%u.%u\n", majorVersion, minorVersion, patchVersion);

        //* Step - 3.4
        printf("Device Name : %s\n", vkPhysicalDeviceProperties.deviceName);

        //* Step - 3.5
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                printf("Device Type : Integrated GPU (iGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                printf("Device Type : Discrete GPU (dGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                printf("Device Type : Virtual GPU (vGPU)\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                printf("Device Type : CPU\n");
            break;

            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                printf("Device Type : Other\n");
            break;

            default:
                printf("Device Type : UNKNOWN\n");
            break;
        }

        //* Step - 3.6
        printf("Device ID : 0x%4x\n", vkPhysicalDeviceProperties.deviceID);

        //* Step - 3.7
        printf("Vendor ID : 0x%4x\n", vkPhysicalDeviceProperties.vendorID);

        switch(vkPhysicalDeviceProperties.vendorID)
        {
            case 0x10DE: printf("Vendor Name : NVIDIA\n"); break;
            case 0x1002: printf("Vendor Name : AMD\n"); break;
            case 0x8086: printf("Vendor Name : Intel\n"); break;
            case 0x106b: printf("Vendor Name : Apple\n"); break;
            default: printf("Vendor Name : Unknown (0x%4x)\n", vkPhysicalDeviceProperties.vendorID);
        }

        //* Additional Properties
        uint32_t queueCount = UINT32_MAX;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
        printf("\nNo. of Queue Families: %d\n", queueCount);

        VkQueueFamilyProperties* vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(queueCount * sizeof(VkQueueFamilyProperties));
        if (vkQueueFamilyProperties_array == NULL)
        {
            printf("%s() => malloc() Failed For vkQueueFamilyProperties_array !!!\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
        if (isQueueSurfaceSupported_array == NULL)
        {
            printf("%s() => malloc() Failed For isQueueSurfaceSupported_array\n", __func__);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        for (uint32_t j = 0; j < queueCount; j++)
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);

        for (uint32_t j = 0; j < queueCount; j++)
        {

            printf("\nQueue Family : %d\n", j);
            printf("****************************************\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                printf("Supports Graphics : Yes\n");
            else
                printf("Supports Graphics : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
                printf("Supports Compute : Yes\n");
            else
                printf("Supports Compute : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
                printf("Supports Transfer Operations : Yes\n");
            else
                printf("Supports Transfer Operations : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                printf("Supports Video Encoding : Yes\n");
            else
                printf("Supports Video Encoding : No\n");

            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                printf("Supports Video Decoding : Yes\n");
            else
                printf("Supports Video Decoding : No\n");

            if (isQueueSurfaceSupported_array[j] == VK_TRUE)
                printf("Supports Presentation : Yes\n");
            else
                printf("Supports Presentation : No\n");
                
            printf("****************************************\n\n");
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

        printf("*******************************************************\n");
    }

    printf("------------------------------------------------------------------------------------------------\n\n");

    //* Step - 3.8
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        printf("%s() => free() Succeeded For vkPhysicalDevice_array\n", __func__);
        vkPhysicalDevice_array = NULL;
    }
    return vkResult;
}

-(VkResult) fillDeviceExtensionNames
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t deviceExtensionCount = 0;
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 1 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 2
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        printf("%s() => malloc() Failed For vkExtensionProperties_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 2 : vkEnumerateDeviceExtensionProperties() Succeeded\n", __func__);

    //* Step - 3
    char **deviceExtensionNames_array = NULL;
    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    if (deviceExtensionNames_array == NULL)
    {
        printf("%s() => malloc() Failed For deviceExtensionNames_array !!!\n", __func__);
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
            printf("%s() => malloc() Failed For deviceExtensionNames_array[%d] !!!\n", __func__, i);
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

        printf("%s() => Vulkan Device Extension Name : %s\n", __func__, deviceExtensionNames_array[i]);
    }

    printf("\n------------------------------------------------------------------------------------------------\n");
    printf("%s() => Vulkan Device Extension Count : %d\n", __func__, deviceExtensionCount);
    printf("------------------------------------------------------------------------------------------------\n\n");

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

    VkBool32 vulkanPortabilitySubsetExtensionFound = VK_FALSE;
    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if (strcmp(deviceExtensionNames_array[i], VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0)
        {
            vulkanPortabilitySubsetExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME;
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
        printf("%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_KHR_SWAPCHAIN_EXTENSION_NAME Extension Found\n", __func__);

    if (vulkanPortabilitySubsetExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        printf("%s() => VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME Extension Not Found !!!\n", __func__);
        return vkResult;
    }
    else
        printf("%s() => VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME Extension Found\n", __func__);

    //* Step - 8
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
        printf("%s() => Enabled Vulkan Device Extension Name : %s\n", __func__, enabledDeviceExtensionNames_array[i]);

    return vkResult;

}

-(VkResult) createVulkanDevice
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;
    float queuePriorities[1] = { 1.0f };

    // Code

    //* Step - 1
    vkResult = [self fillDeviceExtensionNames];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => fillDeviceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
        printf("%s() => fillDeviceExtensionNames() Succeeded\n", __func__);

    
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
        printf("%s() => vkCreateDevice() Failed : %d !!!\n", __func__, vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
        printf("%s() => vkCreateDevice() Succeeded\n", __func__);

    return vkResult;
}

-(void) getDeviceQueue
{
    // Code
    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if (vkQueue == VK_NULL_HANDLE)
    {
        printf("%s() => vkGetDeviceQueue() returned NULL for vkQueue !!!\n", __func__);
        return;
    }
    else
        printf("%s() => vkGetDeviceQueue() Succeeded ...\n", __func__);
}

-(VkResult) getPhysicalDeviceSurfaceFormatAndColorSpace
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t formatCount = 0;
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (formatCount == 0)
    {
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

    //* Step - 2
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    if (vkSurfaceFormatKHR_array == NULL)
    {
        printf("%s() => malloc() Failed For vkSurfaceFormatKHR_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

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
        printf("%s() => free() Succeeded For vkSurfaceFormatKHR_array\n", __func__);
    }

    return vkResult;
}

-(VkResult) getPhysicalDevicePresentMode
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    uint32_t presentModeCount = 0;
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (presentModeCount == 0)
    {
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Returned 0 Devices !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Call 1 : vkGetPhysicalDeviceSurfacePresentModesKHR() Succeeded\n", __func__);

    //* Step - 2
    VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    if (vkPresentModeKHR_array == NULL)
    {
        printf("%s() => malloc() Failed For vkPresentModeKHR_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 2 : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 2 : vkGetPhysicalDeviceSurfacePresentModesKHR() Succeeded\n", __func__);


    //* Step - 4
    for (uint32_t i = 0; i < presentModeCount; i++)
    {
        if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            printf("\n------------------------------------------------------------------------------------------------\n");
            printf("Vulkan Physical Device Present Mode : VK_PRESENT_MODE_MAILBOX_KHR");
            printf("\n------------------------------------------------------------------------------------------------\n\n");
            break;
        }
    }

    if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        printf("\n------------------------------------------------------------------------------------------------\n");
        printf("Vulkan Physical Device Present Mode : VK_PRESENT_MODE_FIFO_KHR");
        printf("\n------------------------------------------------------------------------------------------------\n\n");
    }
        

    //* Step - 5
    if (vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        printf("%s() => free() Succeeded For vkPresentModeKHR_array\n", __func__);
    }

    return vkResult;
}

-(VkResult) createSwapchain:(VkBool32) vsync
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    vkResult = [self getPhysicalDeviceSurfaceFormatAndColorSpace];
    if (vkResult != VK_SUCCESS)
        printf("%s() => getPhysicalDeviceSurfaceFormatAndColorSpace() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => getPhysicalDeviceSurfaceFormatAndColorSpace() Succeeded\n", __func__);

    //* Step - 2
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Succeeded\n", __func__);

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
        printf("%s() => [If Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // If surface size is already defined, then swapchain image size must match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_swapchain.width = MAX(
            vkSurfaceCapabilitiesKHR.minImageExtent.width,
            MIN(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)
        );

        vkExtent2D_swapchain.height = MAX(
            vkSurfaceCapabilitiesKHR.minImageExtent.height,
            MIN(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)
        );

        printf("%s() => [Else Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
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
    vkResult = [self getPhysicalDevicePresentMode];
    if (vkResult != VK_SUCCESS)
        printf("%s() => getPhysicalDevicePresentMode() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => getPhysicalDevicePresentMode() Succeeded\n", __func__);

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
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkCreateSwapchainKHR() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkCreateSwapchainKHR() Succeeded\n", __func__);

    return VK_SUCCESS;
}

-(VkResult) createImagesAndImageViews
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Step - 1
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 1 : vkGetSwapchainImagesKHR() Failed : %d !!!\n", __func__, vkResult);
    else if (swapchainImageCount == 0)
    {
        printf("%s() => Call 1 : vkGetSwapchainImagesKHR() Returned 0 Images !!!\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Call 1 : vkGetSwapchainImagesKHR() => Swapchain Image Count = %d\n", __func__, swapchainImageCount);


    //* Step - 2
    swapchainImage_array = (VkImage*)malloc(swapchainImageCount * sizeof(VkImage));
    if (swapchainImage_array == NULL)
    {
        printf("%s() => malloc() Failed For swapchainImage_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (vkResult != VK_SUCCESS)
        printf("%s() => Call 2 : vkGetSwapchainImagesKHR() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => Call 2 : vkGetSwapchainImagesKHR() Succeeded\n", __func__);


    //* Step - 4
    swapchainImageView_array = (VkImageView*)malloc(swapchainImageCount * sizeof(VkImageView));
    if (swapchainImageView_array == NULL)
    {
        printf("%s() => malloc() Failed For swapchainImageView_array !!!\n", __func__);
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
            printf("%s() => vkCreateImageView() Failed For Index : %d, Error Code : %d !!!\n", __func__, i, vkResult);
        else
            printf("%s() => vkCreateImageView() Succeeded For Index : %d\n", __func__, i);
    }

    return vkResult;
}

-(VkResult) createCommandPool
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
        printf("%s() => vkCreateCommandPool() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateCommandPool() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) createCommandBuffers
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

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
        printf("%s() => malloc() Failed For vkCommandBuffer_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 3
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
            printf("%s() => vkAllocateCommandBuffers() Failed For Index : %d, Error Code : %d !!!\n", __func__, i, vkResult);
        else
            printf("%s() => vkAllocateCommandBuffers() Succeeded For Index : %d\n", __func__, i);
    }

    return vkResult;
}

-(VkResult) createVertexBuffer
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 3
    float rectangle_position[] =
    {
        // Triangle 1
        1.0f,   1.0f,   0.0f,   // Top Right
        -1.0f,  1.0f,   0.0f,   // Top Left
        -1.0f,  -1.0f,  0.0f,   // Bottom Left

        // Triangle 2
        -1.0f,  -1.0f,  0.0f,   // Bottom Left
        1.0f,   -1.0f,  0.0f,   // Bottom Right
        1.0f,   1.0f,   0.0f,   // Top Right
    };

    // Code
    
    //* Step - 4
    memset((void*)&vertexData_position, 0, sizeof(VertexData));

    //* Step - 5
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.flags = 0;   //! Valid Flags are used in sparse(scattered) buffers
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.size = sizeof(rectangle_position);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    //* Step - 6
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkCreateBuffer() Failed For Vertex Buffer : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateBuffer() Succeeded For Vertex Buffer\n", __func__);
    
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
    VkBool32 foundMatchingMemoryType_vertex = VK_FALSE;
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
                foundMatchingMemoryType_vertex = VK_TRUE;
                break;
            }
        }

        //* Step - 8.5
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //* Check For memoryTypeIndex != 0 On MoltenVK
    if (foundMatchingMemoryType_vertex == VK_FALSE)
    {
        vkResult = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        printf("%s() => Host Visible Memory Not Found : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //* Step - 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkAllocateMemory() Failed For Vertex Buffer : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkAllocateMemory() Succeeded For Vertex Buffer\n", __func__);

    //* Step - 10
    //! Binds Vulkan Device Memory Object Handle with the Vulkan Buffer Object Handle
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkBindBufferMemory() Failed For Vertex Buffer : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkBindBufferMemory() Succeeded For Vertex Buffer\n", __func__);

    //* Step - 11
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkMapMemory() Failed For Vertex Buffer : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkMapMemory() Succeeded For Vertex Buffer\n", __func__);

    //* Step - 12
    memcpy(data, rectangle_position, sizeof(rectangle_position));

    //* Step - 13
    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    return vkResult;
}

-(VkResult) createUniformBuffer
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.size = sizeof(MVP_UniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    memset((void*)&uniformData, 0, sizeof(UniformData));

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkCreateBuffer() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkCreateBuffer() Succeeded For Uniform Data\n", __func__);
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    VkBool32 foundMatchingMemoryType_uniform = VK_FALSE;
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                foundMatchingMemoryType_uniform = VK_TRUE;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //* Check For memoryTypeIndex != 0 On MoltenVK
    if (foundMatchingMemoryType_uniform == VK_FALSE)
    {
        vkResult = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        printf("%s() => Host Visible Memory Not Found : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkAllocateMemory() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkAllocateMemory() Succeeded For Uniform Data\n", __func__);

    vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkBindBufferMemory() Failed For Uniform Data : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkBindBufferMemory() Succeeded For Uniform Data\n", __func__);

    vkResult = [self updateUniformBuffer];
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => updateUniformBuffer() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => updateUniformBuffer() Succeeded\n", __func__);


    return vkResult;
}

-(VkResult) updateUniformBuffer
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    MVP_UniformData mvp_UniformData;
    memset((void*)&mvp_UniformData, 0, sizeof(MVP_UniformData));

    //! Update Matrices
    mvp_UniformData.modelMatrix = glm::mat4(1.0f);
    mvp_UniformData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    mvp_UniformData.viewMatrix = glm::mat4(1.0f);
    
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);
    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float)winWidth / (float)winHeight,
        0.1f,
        100.0f
    );
    //! 2D Matrix with Column Major (Like OpenGL)
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f);
    mvp_UniformData.projectionMatrix = perspectiveProjectionMatrix;

    //! Map Uniform Buffer
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(MVP_UniformData), 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkMapMemory() Failed For Uniform Buffer : %d !!!\n", __func__, vkResult);
        return vkResult;
    }

    //! Copy the data to the mapped buffer (present on device memory)
    memcpy(data, &mvp_UniformData, sizeof(MVP_UniformData));

    //! Unmap memory
    vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

    return vkResult;
}

-(VkResult) createShaders
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //! Vertex Shader
    //! ---------------------------------------------------------------------------------------------------------------------------
    //* Step - 6
    NSBundle* appBundle = [NSBundle mainBundle];

    const char* szFileName = "Shader.vert.spv";
    const char* pszShaderFileNameWithPath = [[[appBundle resourcePath]stringByAppendingPathComponent:@(szFileName)]cStringUsingEncoding:NSASCIIStringEncoding];

    FILE *fp = NULL;
    size_t size;

    fp = fopen(pszShaderFileNameWithPath, "rb");
    if (fp == NULL)
    {
        printf("%s() => Failed To Open SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Succeeded In Opening SPIR-V Shader File : %s\n", __func__, szFileName);

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    if (size == 0)
    {
        printf("%s() => Empty SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    fseek(fp, 0L, SEEK_SET);

    char* shaderData = (char*)malloc(size * sizeof(char));
    if (shaderData == NULL)
    {
        printf("%s() => malloc() Failed For shaderData !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    size_t retVal = fread(shaderData, size, 1, fp);
    if (retVal != 1)
    {
        printf("%s() => Failed To Read From SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Successfully Read Shader From SPIR-V Shader File : %s\n", __func__, szFileName);
    
    if (fp)
    {
        fclose(fp);
        fp = NULL;
        printf("%s() => Closed SPIR-V File : %s\n", __func__, szFileName);
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
        printf("%s() => vkCreateShaderModule() Failed For Vertex Shader : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateShaderModule() Succeeded For Vertex Shader\n", __func__);

    //* Step - 9
    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        printf("%s() => free() Succeeded For shaderData\n", __func__);
    }

    printf("%s() => Vertex Shader Module Successfully Created\n", __func__);
    //! ---------------------------------------------------------------------------------------------------------------------------

    //! Fragment Shader
    //! ---------------------------------------------------------------------------------------------------------------------------
    szFileName = "Shader.frag.spv";
    pszShaderFileNameWithPath = [[[appBundle resourcePath]stringByAppendingPathComponent:@(szFileName)]cStringUsingEncoding:NSASCIIStringEncoding];

    fp = NULL;
    fp = fopen(pszShaderFileNameWithPath, "rb");
    if (fp == NULL)
    {
        printf("%s() => Failed To Open SPIR-V Shader File :  %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Succeeded In Opening SPIR-V Shader File : %s\n", __func__, szFileName);

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    if (size == 0)
    {
        printf("%s() => Empty SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    fseek(fp, 0L, SEEK_SET);

    shaderData = (char*)malloc(size * sizeof(char));
    if (shaderData == NULL)
    {
        printf("%s() => malloc() Failed For shaderData !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    retVal = fread(shaderData, size, 1, fp);
    if (retVal != 1)
    {
        printf("%s() => Failed To Read From SPIR-V Shader File : %s !!!", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
        printf("%s() => Successfully Read Shader From SPIR-V Shader File : %s\n", __func__, szFileName);
    
    if (fp)
    {
        fclose(fp);
        fp = NULL;
        printf("%s() => Closed SPIR-V File : %s\n", __func__, szFileName);
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
        printf("%s() => vkCreateShaderModule() Failed For Fragment Shader : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateShaderModule() Succeeded For Fragment Shader\n", __func__);

    //* Step - 9
    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        printf("%s() => free() Succeeded For shaderData\n", __func__);
    }

    printf("%s() => Fragment Shader Module Successfully Created\n", __func__);
    //! ---------------------------------------------------------------------------------------------------------------------------

    return vkResult;
}

-(VkResult) createDescriptorSetLayout
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //! Initialize VkDescriptorSetLayoutBinding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
    memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding.binding = 0;   //! Mapped with layout(binding = 0) in vertex shader
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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
        printf("%s() => vkCreateDescriptorSetLayout() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateDescriptorSetLayout() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) createPipelineLayout
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
        printf("%s() => vkCreatePipelineLayout() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreatePipelineLayout() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) createDescriptorPool
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    // Code

    //* Vulkan expects decriptor pool size before creating actual descriptor pool
    VkDescriptorPoolSize vkDescriptorPoolSize;
    memset((void*)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));
    vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize.descriptorCount = 1;
   
    //* Create the pool
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = 1;
    vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
    vkDescriptorPoolCreateInfo.maxSets = 1;

    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkCreateDescriptorPool() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateDescriptorPool() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) createDescriptorSet
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

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
        printf("%s() => vkAllocateDescriptorSets() Failed : %d !!!\n", __func__, vkResult);
        return vkResult;
    }
    else
        printf("%s() => vkAllocateDescriptorSets() Succeeded\n", __func__);
    
    //* Describe whether we want buffer as uniform or image as uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MVP_UniformData);

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

-(VkResult) createRenderPass
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    VkAttachmentDescription vkAttachmentDescription_array[1];
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

    //* Step - 2
    //! Color Attachment Reference
    VkAttachmentReference vkAttachmentReference;
    memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference.attachment = 0;   //* 0 specifies 0th index in above array
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //* Step - 3
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1;  //! This should be the count of vkAttachmentReference used for color
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
    vkSubpassDescription.pPreserveAttachments = NULL;
    vkSubpassDescription.pResolveAttachments = NULL;

    //! Explicit subpass dependencies are defined below to prevent synchronization validation errors.
    //! While the application may function without them and bypass core validation,
    //! the Synchronization Validation Layer will flag the missing explicit setup.
    VkSubpassDependency vkSubpassDependency_array[1];
    memset((void*)vkSubpassDependency_array, 0, sizeof(VkSubpassDependency) * _ARRAYSIZE(vkSubpassDependency_array));
    
    //! Color Subpass Dependency
    vkSubpassDependency_array[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    vkSubpassDependency_array[0].dstSubpass = 0;
    vkSubpassDependency_array[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency_array[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency_array[0].srcAccessMask = 0;
    vkSubpassDependency_array[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    vkSubpassDependency_array[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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
    vkRenderPassCreateInfo.dependencyCount = _ARRAYSIZE(vkSubpassDependency_array);
    vkRenderPassCreateInfo.pDependencies = vkSubpassDependency_array;

    //* Step - 5
    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
    if (vkResult != VK_SUCCESS)
        printf("%s() => vkCreateRenderPass() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateRenderPass() Succeeded\n", __func__);

    return vkResult;
}

-(VkResult) createPipeline
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Code

    //! Vertex Input State
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //! Rasterization State
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
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
        printf("%s() => vkCreatePipelineCache() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreatePipelineCache() Succeeded\n", __func__);

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
    vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
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
        printf("%s() => vkCreateGraphicsPipelines() Failed : %d !!!\n", __func__, vkResult);
    else
        printf("%s() => vkCreateGraphicsPipelines() Succeeded\n", __func__);

    //* Destroy Pipeline Cache
    if (vkPipelineCache)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        printf("%s() => vkDestroyPipelineCache() Succeeded\n", __func__);
    }

    return vkResult;
}

-(VkResult) createFramebuffers
{
    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

    //* Step - 1
    VkImageView vkImageView_attachments_array[1];
    memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

    //* Step - 2
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

    //* Step - 3
    vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    if (vkFramebuffer_array == NULL)
    {
        printf("%s() => malloc() Failed For vkFramebuffer_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 2
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageView_attachments_array[0] = swapchainImageView_array[i];

        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => vkCreateFramebuffer() Failed For Index : %d, Reason : %d !!!\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
    }

    return vkResult;
}

-(VkResult) createSemaphores
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
        printf("%s() => vkCreateSemaphore() Failed For vkSemaphore_backBuffer : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);
    if (vkResult != VK_SUCCESS)
    {
        printf("%s() => vkCreateSemaphore() Failed For vkSemaphore_renderComplete : %d !!!\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    return vkResult;
}

-(VkResult) createFences
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
        printf("%s() => malloc() Failed For vkFence_array !!!\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    //* Step - 6
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => vkCreateFence() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        else
            printf("%s() => vkCreateFence() Succeeded For Index : %d\n", __func__, i);
    }

    return vkResult;
}

-(VkResult) buildCommandBuffers
{
    // Code
    VkResult vkResult = VK_SUCCESS;

    //! Loop per swapchain image
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        //* Step - 1 => Reset Command Buffer
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);   //! 0 specifies not to release the resources
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => vkResetCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        else
            printf("%s() => vkResetCommandBuffer() Succeeded For Index : %d\n", __func__, i);

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
            printf("%s() => vkBeginCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        else
            printf("%s() => vkBeginCommandBuffer() Succeeded For Index : %d\n", __func__, i);

        //* Step - 4 => Set Clear Value
        VkClearValue vkClearValue_array[1];
        memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));
        vkClearValue_array[0].color = vkClearColorValue;

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

            //! Bind with Vertex Buffer
            VkDeviceSize vkDeviceSize_offset_array[1];
            memset((void*)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_array));
            vkCmdBindVertexBuffers(
                vkCommandBuffer_array[i],
                0,
                1,
                &vertexData_position.vkBuffer,
                vkDeviceSize_offset_array
            );

            //! Vulkan Drawing Function
            vkCmdDraw(vkCommandBuffer_array[i], 6, 1, 0, 0);
        }
        //* Step - 7
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);

        //* Step - 8
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            printf("%s() => vkEndCommandBuffer() Failed For Index : %d, Reason : %d\n", __func__, i, vkResult);
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            return vkResult;
        }
        else
            printf("%s() => vkEndCommandBuffer() Succeeded For Index : %d\n", __func__, i);
    }

    return vkResult;
}


@end



VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT vkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT vkDebugReportObjectTypeEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
    void* pUserData
)
{
    // Code
    printf("ADN_VALIDATION : debugUtilsMessengerCallback() => %s\n : ", pCallBackData->pMessage);
    return VK_FALSE;
}

