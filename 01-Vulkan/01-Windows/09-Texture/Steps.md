# Steps For Vulkan Texture

1. Get image info - width, height, number of channels and actual image data using functions in `stb_image.h`

2. Put above image data into a staging buffer.

3. Create `Device Only Visible`, empty, but enough sized VkImage equal to the size of the image `image_width * image_height`.

4. Send `Image Layout Transition` to the GPU before the actual staging buffer from `Step-2` to empty VkImage from `Step-3` using `Pipeline Barrier`

5. Now, actually copy the image data from staging buffer to the empty VkImage.

6. Again perform `Image Layout Transition` similar to `Step-4` for correct reading/writing of image data by the shaders.

7. Now, staging buffer is not needed. Hence, release its memory and itself.

8. Create `Image View` for above image.

9. Create `Texture Sampler` for above image.

10. Uninitialize texture related all global data - 
    - VkImage
    - VkDeviceMemory of image
    - VkImageView of image
    - Sampler