#!/bin/bash

clear

VULKAN_INCLUDE_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/include"
VULKAN_LIB_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/lib"
VULKAN_BIN_PATH="/home/atharv/VulkanSDK/Vulkan/x86_64/bin"

GLM_INCLUDE_PATH="/home/atharv/Desktop/Workspace/ARTR/01-Vulkan/02-Linux/External/glm"

API="Vulkan"
SPV=1

rm -f *.o 

echo "--------------------------------------------------------------------------------"
echo "Compiling X11 and $API Source Code ..."
echo "--------------------------------------------------------------------------------"
g++ -c -o Vk.o -I $VULKAN_INCLUDE_PATH -I $GLM_INCLUDE_PATH Vk.cpp

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
        mv Bin/Shader.vert.spv ../
        mv Bin/Shader.frag.spv ../
    cd ..
fi

echo
echo "--------------------------------------------------------------------------------"
echo "Linking Object Files and Creating Binary ..."
echo "--------------------------------------------------------------------------------"

g++ Vk.o -o Vk -lX11 -lm -L$VULKAN_LIB_PATH -lvulkan

if [ $? -ne 0 ]; then
    echo "Linking Failed !!!"
    exit 1
fi

echo
echo "--------------------------------------------------------------------------------"
echo "Launching Application ..."
echo "--------------------------------------------------------------------------------"
./Vk


