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
cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\Include Vk.c

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
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert
    C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag
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
