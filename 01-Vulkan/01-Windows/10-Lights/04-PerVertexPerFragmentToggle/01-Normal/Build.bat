@echo off

set API=Vulkan
set SPV=1

cls

if exist *.obj del *.obj
if exist *.exe del *.exe
if exist *.res del *.res

echo ----------------------------------------------------------------------------------------------------------------
echo Compiling %API% and Win32 Source Code ...
echo ----------------------------------------------------------------------------------------------------------------
cl.exe  /c ^
        /EHsc ^
        /std:c++17 ^
        /I C:\VulkanSDK\Vulkan\Include ^
        /I C:\VulkanSDK\Vulkan\Include\glm ^
        Vk.cpp

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Compiling Resource Files ...
echo ----------------------------------------------------------------------------------------------------------------
rc.exe Vk.rc

@echo:
if %SPV%==1 (
   echo ----------------------------------------------------------------------------------------------------------------
    echo Compiling Shader Files To SPIR-V Binaries ...
    echo ----------------------------------------------------------------------------------------------------------------
    cd Shaders
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o PerVertexShader.vert.spv PerVertexShader.vert
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o PerVertexShader.frag.spv PerVertexShader.frag
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o PerFragmentShader.vert.spv PerFragmentShader.vert
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o PerFragmentShader.frag.spv PerFragmentShader.frag
    move PerVertexShader.vert.spv ../
    move PerVertexShader.frag.spv ../
    move PerFragmentShader.vert.spv ../
    move PerFragmentShader.frag.spv ../
    cd ..
)

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Linking Libraries and Resources...
echo Creating Executable...
echo ----------------------------------------------------------------------------------------------------------------
link.exe Vk.obj Vk.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Launching Application ...
echo ----------------------------------------------------------------------------------------------------------------
Vk.exe
