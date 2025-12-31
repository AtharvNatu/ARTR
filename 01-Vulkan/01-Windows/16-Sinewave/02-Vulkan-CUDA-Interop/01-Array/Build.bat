
@echo off

set API=Vulkan

set VULKAN_INCLUDE_PATH=C:\VulkanSDK\Vulkan\Include
set VULKAN_LIB_PATH=C:\VulkanSDK\Vulkan\Lib
set VULKAN_BIN_PATH=C:\VulkanSDK\Vulkan\Bin

set SOURCE_PATH=Source
set INCLUDE_PATH=Include
set IMAGES_PATH=Assets\Images

set BIN_DIR=Bin
set SPV=1

cls

if not exist %BIN_DIR% mkdir %BIN_DIR%

cls

if exist %BIN_DIR%\*.obj del /q %BIN_DIR%\*.obj >nul 2>&1
if exist %BIN_DIR%\*.exe del /q %BIN_DIR%\*.res >nul 2>&1
if exist %BIN_DIR%\*.res del /q %BIN_DIR%\*.exe >nul 2>&1


@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Compiling Resource Files ...
echo ----------------------------------------------------------------------------------------------------------------
rc.exe ^
        /I %INCLUDE_PATH% ^
        /I %IMAGES_PATH%^
        /fo %BIN_DIR%\Vk.res ^
        Assets\Vk.rc

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
    move Shader.vert.spv ../%BIN_DIR%
    move Shader.frag.spv ../%BIN_DIR%
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
nvcc.exe -w -o %BIN_DIR%/Vk.exe ^
        -I%INCLUDE_PATH% ^
        -I%VULKAN_INCLUDE_PATH% ^
        -I%VULKAN_INCLUDE_PATH%\glm ^
        -L%VULKAN_LIB_PATH% ^
        -Xcompiler="/EHsc" ^
        -Wno-deprecated-gpu-targets ^
        gdi32.lib user32.lib ^
        %BIN_DIR%\Vk.res ^
        %SOURCE_PATH%\Vk.cu

if errorlevel 1 (
        @echo:
        echo Compilation Failed !!!
        exit /b 1
)


@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Launching Application ...
echo ----------------------------------------------------------------------------------------------------------------
cd %BIN_DIR%
Vk.exe
cd ..

