clear

SOURCE_PATH=Source
BIN_DIR=Bin
SPV=1

ARCH=$(sysctl -n hw.machine 2>/dev/null || uname -m)

VULKAN_INCLUDE_PATH="$HOME/VulkanSDK/Vulkan/macOS/include"
VULKAN_LIB_PATH="$HOME/VulkanSDK/Vulkan/macOS/lib"
VULKAN_FRAMEWORK_PATH="$HOME/VulkanSDK/Vulkan/macOS/Frameworks"

GLM_INCLUDE_PATH="$HOME/VulkanSDK/Vulkan/macOS/include"

echo
echo "--------------------------------------------------------------------------------"
echo "Creating Directory Layout ..."
echo "--------------------------------------------------------------------------------"
mkdir -p "$BIN_DIR"

rm -rf "$BIN_DIR/Vk.app"
mkdir -p "$BIN_DIR/Vk.app/Contents/MacOS"
mkdir -p "$BIN_DIR/Vk.app/Contents/Resources"
cp Assets/* "$BIN_DIR/Vk.app/Contents/Resources"

echo
echo "--------------------------------------------------------------------------------"
echo "Compiling Cocoa + MoltenVk Source Code And Linking Frameworks ..."
echo "--------------------------------------------------------------------------------"
clang++ \
    -Wno-deprecated-declarations \
    -arch "$ARCH" \
    -I"$GLM_INCLUDE_PATH" \
    -I"$VULKAN_INCLUDE_PATH" \
    -L"$VULKAN_LIB_PATH" \
    -F"$VULKAN_FRAMEWORK_PATH" \
    -rpath "$VULKAN_FRAMEWORK_PATH" \
    -o "$BIN_DIR/Vk.app/Contents/MacOS/Vk" \
    "$SOURCE_PATH/Vk.mm" \
    -framework QuartzCore \
    -framework Cocoa \
    -framework vulkan
    
if [ $? -ne 0 ]; then
    echo "Compilation Failed !!!"
    exit 1
fi

if (( SPV == 1)); then
    echo
    echo "--------------------------------------------------------------------------------"
    echo "Compiling Shader Files To SPIR-V Binaries ..."
    echo "--------------------------------------------------------------------------------"
    cd Shaders
        glslangValidator -V -H -o Shader.vert.spv Shader.vert
        glslangValidator -V -H -o Shader.frag.spv Shader.frag
        if [ $? -ne 0 ]; then
            echo "Shader Compilation Failed !!!"
            exit 1
        fi
        mv Shader.vert.spv ../"$BIN_DIR"
        mv Shader.frag.spv ../"$BIN_DIR"
    cd ..
fi

echo
echo "--------------------------------------------------------------------------------"
echo "Opening Application ..."
echo "--------------------------------------------------------------------------------"
open "$BIN_DIR/Vk.app"



