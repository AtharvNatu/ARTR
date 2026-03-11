.\gradlew.bat clean

.\gradlew.bat build

adb install app/build/outputs/apk/debug/app-debug.apk

@REM adb logcat | findstr /i "ADN:"