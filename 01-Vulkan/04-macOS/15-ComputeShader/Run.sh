clear

BIN_DIR=Bin

VULKAN_PATH="$HOME/VulkanSDK/Vulkan"
export VK_ICD_FILENAMES="$VULKAN_PATH/macOS/share/vulkan/icd.d/MoltenVK_icd.json"
export VK_LAYER_PATH="$VULKAN_PATH/macOS/share/vulkan/explicit_layer.d"

./Bin/Vk.app/Contents/MacOS/Vk
