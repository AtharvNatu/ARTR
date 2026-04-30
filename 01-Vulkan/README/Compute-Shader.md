# Changes For Compute Shader

## Global Declarations

```cpp
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

VkCommandBuffer vkCommandBuffer_compute = VK_NULL_HANDLE;               // For Storing Dispatch Command and Barriers
VkCommandBuffer vkCommandBuffer_array_compute[2];                       // For Storing Compute Command Buffer Handle and Graphics Command Buffer Handle

VkShaderModule vkShaderModule_compute_shader = VK_NULL_HANDLE;
VkDescriptorSetLayout vkDescriptorSetLayout_compute = VK_NULL_HANDLE;   // Compute Shader Binding Layout
VkDescriptorSet vkDescriptorSet_compute = VK_NULL_HANDLE;               // Compute Shader Bindings
VkPipelineLayout vkPipelineLayout_compute = VK_NULL_HANDLE;             // Compute Pipeline Layout
VkPipeline vkPipeline_compute = VK_NULL_HANDLE;                         // Compute Pipeline for Sinewave

ComputeData computeBuffer;
Bool bUseCompute = False;
```

## Compute Shader - Sinewave (Shader.comp)

```glsl
#version 460 core

layout(local_size_x = 16, local_size_y = 16) in;

//* Output Buffer
layout(std430, binding = 1) buffer positionBuffer
{
    vec4 position[];
};

//* Push Constants
layout(push_constant) uniform PushConstants
{
    int width;
    int height;
    float time;
} pushData;

void main()
{
    // Code
    uint i = gl_GlobalInvocationID.x;
    uint j = gl_GlobalInvocationID.y;

    if (i >= pushData.width || j >= pushData.height)
        return;

    float u = float(i) / float(pushData.width);
    float v = float(j) / float(pushData.height);

    u = u * 2.0 - 1.0;
    v = v * 2.0 - 1.0;

    float frequency = 4.0;

    float w = sin(u * frequency + pushData.time) * cos(v * frequency + pushData.time) * 0.5;

    position[j * pushData.width + i] = vec4(u, w, v, 1.0);
}
```


## initialize() - Function Declaration and Call

```cpp
//! Compute Related
VkResult initialize_compute(void);
```

### AFTER Error Checking For buildCommandBuffers()

```cpp
vkResult = initialize_compute();
if (vkResult != VK_SUCCESS)
{
    fprintf(gpFile, "%s() => initialize_compute() Failed\n", __func__);
    vkResult = VK_ERROR_INITIALIZATION_FAILED;
    return vkResult;
}
else
    fprintf(gpFile, "%s() => initialize_compute() Succeeded\n", __func__);
```


## display()


```cpp
VkResult buildCommandBuffer_compute(void);
```

### AFTER Error Checking For buildCommandBuffers()

```cpp
if (bUseCompute)
{
    vkResult = buildCommandBuffer_compute();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s() => buildCommandBuffer_compute() Failed : %d\n", __func__, vkResult);
        return vkResult;
    }

    vkCommandBuffer_array_compute[0] = vkCommandBuffer_compute;
    vkCommandBuffer_array_compute[1] = vkCommandBuffer_array[currentImageIndex];
}
```

### AFTER Declaration of waitDstStageMask

```cpp
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
    vkSubmitInfo.commandBufferCount = 2;
    vkSubmitInfo.pCommandBuffers = vkCommandBuffer_array_compute;
}
else
{
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
}
```

## uninitialize() -  Function Declaration and Call

```cpp
void uninitialize_compute(void);
```

### AFTER vkDeviceWaitIdle()

```cpp
uninitialize_compute();
```

## Descriptor Pool Modifications For Compute SSBO

```cpp
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
        fprintf(gpFile, "%s() => vkCreateDescriptorPool() Failed : %d !!!\n", __func__, vkResult);  
    else
        fprintf(gpFile, "%s() => vkCreateDescriptorPool() Succeeded\n", __func__);

    return vkResult;
}
```

### Add Functions to code with names containing _compute()