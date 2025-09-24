## Push Constants

- Have to enable Push Constant Ranges before creating pipeline.
- Such constants which are small values, capable of updating in the shader without using UBO like large sized objects
- But, limit of push constant is implmentation-dependent or hardware dependent. Vulkan spec guarentees to allow `minimum` 128 bytes currently. The maximum value is varying, but generally 256 bytes is considered okay.
- For larger values, the best way is to get the maximum limit at runtime using `VkGetPhysicalDeviceProperties()` to return `struct VkPhysicalDeviceProperties`. The `.limits` member is of type `VkPhysicalDeviceLimits`.
- The `.maxPushConstantsSize` will give you the actual limit of Push constants.
- Pipeline layout must be compatible for push constant because pipeline layout depends upon supported queue family and push constants depend upon such supported queue family.