#!/bin/bash

clear

API="Vulkan"

VULKAN_INCLUDE_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/include"
VULKAN_LIB_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/lib"
VULKAN_BIN_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/bin"

GLM_INCLUDE_PATH="/home/atharv/Desktop/Workspace/ARTR/01-Vulkan/02-Linux/External/glm"

SOURCE_PATH=Source
INCLUDE_PATH=Include

BIN_DIR=Bin

mkdir -p "$BIN_DIR"

clear

rm -f "$BIN_DIR"/*.o 2>/dev/null

echo "--------------------------------------------------------------------------------"
echo "Compiling X11 and $API Source Code ..."
echo "--------------------------------------------------------------------------------"
gcc -c \
    -o Vk.o \
    -I"$VULKAN_INCLUDE_PATH" \
    -I"$GLM_INCLUDE_PATH" \
    -I"$INCLUDE_PATH" \
    "$SOURCE_PATH/Vk.c"

if [ $? -ne 0 ]; then
    echo "Compilation Failed !!!"
    exit 1
fi

mv *.o "$BIN_DIR" 2>/dev/null

echo
echo "--------------------------------------------------------------------------------"
echo "Linking Object Files and Creating Binary ..."
echo "--------------------------------------------------------------------------------"

gcc "$BIN_DIR"/Vk.o -o "$BIN_DIR"/Vk \
    -lX11 \
    -lm \
    -L"$VULKAN_LIB_PATH" \
    -lvulkan

if [ $? -ne 0 ]; then
    echo "Linking Failed !!!"
    exit 1
fi

echo
echo "--------------------------------------------------------------------------------"
echo "Launching Application ..."
echo "--------------------------------------------------------------------------------"
cd "$BIN_DIR"
./Vk


