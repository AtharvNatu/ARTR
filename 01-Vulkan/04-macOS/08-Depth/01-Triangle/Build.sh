clear

SOURCE_PATH=Source
BIN_DIR=Bin

VULKAN_INCLUDE_PATH="/Users/atharv/VulkanSDK/1.4.304.0/macOS/include"
VULKAN_LIB_PATH="/Users/atharv/VulkanSDK/1.4.304.0/macOS/lib"
VULKAN_BIN_PATH="/Users/atharv/VulkanSDK/1.4.304.0/macOS/bin"

GLM_INCLUDE_PATH=/Users/atharv/VulkanSDK/1.4.304.0/macOS/include

echo "--------------------------------------------------------------------------------"
echo "Creating Directory Layout ..."
echo "--------------------------------------------------------------------------------"
mkdir -p "$BIN_DIR"

rm -rf "$BIN_DIR/Vk.app"
mkdir -p "$BIN_DIR/Vk.app/Contents/MacOS"
mkdir -p "$BIN_DIR/Vk.app/Contents/Resources"
cp Assets/* "$BIN_DIR/Vk.app/Contents/Resources"

echo "--------------------------------------------------------------------------------"
echo "Compiling Cocoa + MoltenVk Source Code And Linking Frameworks ..."
echo "--------------------------------------------------------------------------------"
clang++ \
    -Wno-deprecated-declarations \
    -arch arm64 \
    -I"$GLM_INCLUDE_PATH" \
    -I"$VULKAN_INCLUDE_PATH" \
    -L"$VULKAN_LIB_PATH" \
    -lvulkan \
    -o "$BIN_DIR/Vk.app/Contents/MacOS/Vk" \
    "$SOURCE_PATH/Vk.mm" \
    -framework Foundation \
    -framework Cocoa \
    
# if [ $? -ne 0 ]; then
#     echo "Compilation Failed !!!"
#     exit 1
# else 
#     echo "--------------------------------------------------------------------------------"
#     echo "Opening Application ..."
#     echo "--------------------------------------------------------------------------------"
#     open "$BIN_DIR/Vk.app"
# fi



