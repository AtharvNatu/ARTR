@echo off

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