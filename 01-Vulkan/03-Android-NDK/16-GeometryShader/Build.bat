@echo off

set VULKAN_BIN_PATH="C:\\Users\\Atharv\\AppData\\Local\\Android\\Sdk\\ndk\\29.0.14206865\\shader-tools\\windows-x86_64"
set ASSETS_PATH="%CD%\\app\\src\\main\\assets"
set DEPLOY=1

set BUILD_DIR_1="%CD%\\app\\build"
set BUILD_DIR_2="%CD%\\app\\.cxx"

if exist %BUILD_DIR_1% rmdir /s /q %BUILD_DIR_1%
if exist %BUILD_DIR_2% rmdir /s /q %BUILD_DIR_2%

cls

echo ----------------------------------------------------------------------------------------------------------------
echo Cleaning ...
echo ----------------------------------------------------------------------------------------------------------------

call gradlew.bat clean

if errorlevel 1 (
        @echo:
        echo Clean Failed !!!
        exit /b 1
)

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Compiling Shader Files To SPIR-V Binaries ...
echo ----------------------------------------------------------------------------------------------------------------

%VULKAN_BIN_PATH%\glslc.exe -fshader-stage=vertex --target-env=vulkan1.1 -o %ASSETS_PATH%\Cube.vert.spv %ASSETS_PATH%\Cube.vert        
%VULKAN_BIN_PATH%\glslc.exe -fshader-stage=fragment --target-env=vulkan1.1 -o %ASSETS_PATH%\Cube.frag.spv %ASSETS_PATH%\Cube.frag
%VULKAN_BIN_PATH%\glslc.exe -fshader-stage=vertex --target-env=vulkan1.1 -o %ASSETS_PATH%\Teapot.vert.spv %ASSETS_PATH%\Teapot.vert        
%VULKAN_BIN_PATH%\glslc.exe -fshader-stage=fragment --target-env=vulkan1.1 -o %ASSETS_PATH%\Teapot.frag.spv %ASSETS_PATH%\Teapot.frag

if errorlevel 1 (
        @echo:
        echo Shader Compilation Failed !!!
        exit /b 1
)

@echo:
echo Shader Files Compiled Successfully using glslc.exe from Android NDK ...

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Building Application ...
echo ----------------------------------------------------------------------------------------------------------------

call gradlew.bat build

if errorlevel 1 (
        @echo:
        echo Build Failed !!!
        exit /b 1
)

@echo:
echo ----------------------------------------------------------------------------------------------------------------
echo Deploying APK ...
echo ----------------------------------------------------------------------------------------------------------------

adb install -r app/build/outputs/apk/debug/app-debug.apk

if errorlevel 1 (
        @echo:
        echo APK Deployment Failed !!!
        exit /b 1
)


echo ----------------------------------------------------------------------------------------------------------------
echo Performing Cleanup ...
echo ----------------------------------------------------------------------------------------------------------------

if exist %BUILD_DIR_1% rmdir /s /q %BUILD_DIR_1%
if exist %BUILD_DIR_2% rmdir /s /q %BUILD_DIR_2%



@REM adb logcat | findstr /i "ADN:"