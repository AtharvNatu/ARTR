
@echo off

set API=Vulkan
set VULKAN_INCLUDE_PATH=C:\VulkanSDK\Vulkan\Include
set VULKAN_LIB_PATH=C:\VulkanSDK\Vulkan\Lib
set VULKAN_BIN_PATH=C:\VulkanSDK\Vulkan\Bin
set SPV=0

cls

if exist *.obj del *.obj
if exist *.exe del *.exe
if exist *.res del *.res


@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Compiling Resource Files ...
echo ----------------------------------------------------------------------------------------------------------------
rc.exe Vk.rc

if errorlevel 1 (
        @echo:
        echo Resource Compilation Failed !!!
        exit /b 1
)

@echo:
if %SPV%==1 (
   echo ----------------------------------------------------------------------------------------------------------------
    echo Compiling Shader Files To SPIR-V Binaries ...
    echo ----------------------------------------------------------------------------------------------------------------
    cd Shaders
    %VULKAN_BIN_PATH%\glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert
    %VULKAN_BIN_PATH%\glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag
    move Shader.vert.spv ../
    move Shader.frag.spv ../
    cd ..
    if errorlevel 1 (
        @echo:
        echo Shader Compilation Failed !!!
        exit /b 1
)
)

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Compiling %API% and Win32 Source Code ...
echo Linking Libraries and Resources...
echo Creating Executable...
echo ----------------------------------------------------------------------------------------------------------------
nvcc.exe -w -o Vk.exe ^
        -I%VULKAN_INCLUDE_PATH% ^
        -I%VULKAN_INCLUDE_PATH%\glm ^
        -L%VULKAN_LIB_PATH% ^
        -Xcompiler="/EHsc" ^
        -Wno-deprecated-gpu-targets ^
        gdi32.lib user32.lib ^
        Vk.res Vk.cu

if errorlevel 1 (
        @echo:
        echo Compilation Failed !!!
        exit /b 1
)


@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Launching Application ...
echo ----------------------------------------------------------------------------------------------------------------
Vk.exe
