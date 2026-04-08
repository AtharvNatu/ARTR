#include <android_native_app_glue.h>        // Everything related with pure native activity needs this wrapper
#include <android/log.h>                    // For android_log_print()

#include <memory.h>                         // For memset()
#include <math.h>                           // For sqrtf()

typedef struct 
{
    struct android_app *app;
    bool bActive;
} Engine;

ANativeWindow *androidNativeWindow = NULL;
long touchStartTime = 0;
long pendingSingleTapTime = 0;
bool bTouchDown = false;
bool bDragging = false;
bool bLongPressDetected = false;
bool bDoubleTapDetected = false;
bool bPendingSingleTap = false;
float touchStartX = 0.0f, touchStartY = 0.0f;
float lastTapX = 0.0f, lastTapY = 0.0f;

// Global Callback Function Declarations
void engine_handle_cmd(struct android_app*, int32_t);
int32_t engine_handle_input(struct android_app*, AInputEvent*);

void android_main(struct android_app* state)
{
    // Code

    //* Fullscreen and Hiding Status Bar
    JavaVM *vm = state->activity->vm;
    JNIEnv *env = NULL;

    //* android_main runs on a separate native thread (not the Java UI thread)
    vm->AttachCurrentThread(&env, NULL);

    jobject activityObject = state->activity->clazz;
    jclass activityClass = env->GetObjectClass(activityObject);

    jclass windowClass = env->FindClass("android/view/Window");
    jclass viewClass = env->FindClass("android/view/View");

    jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
    jobject windowObject = env->CallObjectMethod(activityObject, getWindowMethod);

    jmethodID getDecorViewMethod = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorViewObject = env->CallObjectMethod(windowObject, getDecorViewMethod);

    //* Get 8 View Class Static Fields
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_LOW_PROFILE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LOW_PROFILE", "I"));
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I"));

    jmethodID setSystemUiVisibilityMethod = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");
    env->CallVoidMethod(
        decorViewObject, 
        setSystemUiVisibilityMethod, 
        flag_SYSTEM_UI_FLAG_IMMERSIVE |
        flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
        flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_LOW_PROFILE |
        flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY
    );

    //* Notch Fix For Fullscreen
    jclass layoutParamsClass = env->FindClass("android/view/WindowManager$LayoutParams");

    jmethodID getAttributesMethod = env->GetMethodID(windowClass, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
    jobject layoutParams = env->CallObjectMethod(windowObject, getAttributesMethod);

    jfieldID layoutInDisplayCutoutModeField = env->GetFieldID(layoutParamsClass, "layoutInDisplayCutoutMode", "I");
    jfieldID shortEdgesField = env->GetStaticFieldID(layoutParamsClass, "LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES", "I");
    jint shortEdges = env->GetStaticIntField(layoutParamsClass, shortEdgesField);
    env->SetIntField(layoutParams, layoutInDisplayCutoutModeField, shortEdges);

    jmethodID setAttributesMethod = env->GetMethodID(windowClass, "setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V");
    env->CallVoidMethod(windowObject, setAttributesMethod, layoutParams);

    //* Change To Landscape Mode
    jclass activityInfoClass = env->FindClass("android/content/pm/ActivityInfo");
    const int flag_SCREEN_ORIENTATION_LANDSCAPE = env->GetStaticIntField(activityInfoClass, env->GetStaticFieldID(activityInfoClass, "SCREEN_ORIENTATION_LANDSCAPE", "I"));

    jmethodID setRequestedOrientationMethod = env->GetMethodID(activityClass, "setRequestedOrientation", "(I)V");
    env->CallVoidMethod(activityObject, setRequestedOrientationMethod, flag_SCREEN_ORIENTATION_LANDSCAPE);

    //* Detach VM from current thread
    vm->DetachCurrentThread();

    Engine engine;
    memset((void*)&engine, 0, sizeof(Engine));

    // Initialize State
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.app = state;
    __android_log_print(ANDROID_LOG_INFO, "ADN:", "android_main() Started Successfully");

    while (1)
    {
        int identifier;
        struct android_poll_source* source = NULL;

        while ((identifier = ALooper_pollOnce(engine.bActive ? 0 : -1, NULL, NULL, (void**)&source)) >= 0)
        {
            // Process System Events
            if (source != NULL)
            {
                source->process(state, source);
            }

            // Check when to exit
            if (state->destroyRequested)
                return;
        }

        //* Handle Long Press Event
        if (bTouchDown == true && bDragging == false && bLongPressDetected == false && bDoubleTapDetected == false)
        {
            struct timespec ts;
            memset((void*)&ts, 0, sizeof(timespec));
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
            if (now - touchStartTime > 500)
            {
                bLongPressDetected = true;
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Long Press Event");
            }
        }

        //* Handle Single Tap Event
        if (bPendingSingleTap)
        {
            struct timespec ts;
            memset((void*)&ts, 0, sizeof(timespec));
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
            if (now - pendingSingleTapTime > 300)
            {
                bPendingSingleTap = false;
                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Single Tap Event");
            }
        }
    }
}

void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    // Code
    Engine *engine = (Engine*)app->userData;

    switch(cmd)
    {
        case APP_CMD_SAVE_STATE:
            engine->bActive = false;
        break;

        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL)
            {
                androidNativeWindow = engine->app->window;

                //* Draw background color with pixel by pixel coloring using CPU
                ANativeWindow_Buffer buffer;
                uint32_t *pixels = NULL;
                uint32_t color;
                int x, y;

                //* Set buffer geometry and format
                ANativeWindow_setBuffersGeometry(androidNativeWindow, 0, 0, WINDOW_FORMAT_RGBA_8888);

                if (ANativeWindow_lock(androidNativeWindow, &buffer, NULL) == 0)
                {
                    pixels = (uint32_t*)buffer.bits;
                    color = 0xFFFF00FF; // ABGR

                    for (y = 0; y < buffer.height; y++)
                    {
                        for (x = 0; x < buffer.width; x++)
                        {
                            pixels[y * buffer.stride + x] = color;
                        }
                    }

                    ANativeWindow_unlockAndPost(androidNativeWindow);
                }

                __android_log_print(ANDROID_LOG_INFO, "ADN:", "Window Created");

                engine->bActive = true;
            }
            else
                androidNativeWindow = NULL;
        break;

        case APP_CMD_TERM_WINDOW:
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "Window Destroyed");
        break;

        case APP_CMD_GAINED_FOCUS:
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "Window Got Focus");
            engine->bActive = true;
        break;

        case APP_CMD_LOST_FOCUS:
            __android_log_print(ANDROID_LOG_INFO, "ADN:", "Window Lost Focus");
            engine->bActive = false;
        break;
    }

}

int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    // Code
    Engine *engine = (Engine*)app->userData;

    int32_t eventType = AInputEvent_getType(event);
    switch(eventType)
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
            switch(action)
            {
                case AMOTION_EVENT_ACTION_DOWN:
                {
                    touchStartX = AMotionEvent_getX(event, 0);
                    touchStartY = AMotionEvent_getY(event, 0);

                    struct timespec ts;
                    memset((void*)&ts, 0, sizeof(timespec));
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    touchStartTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

                    bTouchDown = true;
                    bDragging = false;
                    bLongPressDetected = false;
                    bDoubleTapDetected = false;

                    //* Check whether the down action is for second tap of the double tap
                    if (bPendingSingleTap)
                    {
                        long timeSinceLastTap = touchStartTime - pendingSingleTapTime;
                        float dx = touchStartX - lastTapX;
                        float dy = touchStartY - lastTapY;
                        float distance = sqrtf(dx * dx + dy * dy);
                        if (timeSinceLastTap < 300 && distance < 100.0f)
                        {
                            //* Double Tap Detected
                            bDoubleTapDetected = true;
                            __android_log_print(ANDROID_LOG_INFO, "ADN:", "Double Tap Event");
                        }
                        else
                        {
                            //* Not a double tap - maybe fling | long tap | single tap  
                        }
                    }

                    bPendingSingleTap = false;
                    break;
                }
                

                case AMOTION_EVENT_ACTION_MOVE:
                {
                    float currentX = AMotionEvent_getX(event, 0);
                    float currentY = AMotionEvent_getY(event, 0);
                    float dx = currentX - touchStartX;
                    float dy = currentY - touchStartY;
                    float distance = sqrtf(dx * dx + dy * dy);
                    if (distance > 50.0f)
                        bDragging = true;
                    break;
                }
                

                case AMOTION_EVENT_ACTION_UP:
                {
                    struct timespec ts;
                    memset((void*)&ts, 0, sizeof(timespec));
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    long currentTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
                    long duration = currentTime - touchStartTime;
                    float endX = AMotionEvent_getX(event, 0);
                    float endY = AMotionEvent_getY(event, 0);
                    float dx = endX - touchStartX;
                    float dy = endY - touchStartY;
                    float distance = sqrtf(dx * dx + dy * dy);
                    bTouchDown = false;
                    if (bDoubleTapDetected)
                    {
                        //* This up belongs to second tap of double tap
                    }
                    else if (bLongPressDetected)
                    {
                        //* This up belongs to long press (already handled) 
                    }
                    else if (bDragging && distance > 150.0f)
                    {
                        __android_log_print(ANDROID_LOG_INFO, "ADN:", "Swipe Event");
                        ANativeActivity_finish(engine->app->activity);
                    }
                    else if (duration < 300.0f && distance < 50.0f)
                    {
                        //* Short tap is detected and second tap may be pending (this may be up of single tap - already handled in main loop)
                        bPendingSingleTap = true;
                        pendingSingleTapTime = currentTime;
                        lastTapX = endX;
                        lastTapY = endY;
                    }
                    bDragging = false;
                    break;
                }
                
            }
            break;
        }
        
    }

    return 0;
}

