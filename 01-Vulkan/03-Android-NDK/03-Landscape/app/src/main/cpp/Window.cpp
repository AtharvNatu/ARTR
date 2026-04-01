#include <android_native_app_glue.h>        // Everything related with pure native activity needs this wrapper
#include <android/log.h>                    // For android_log_print()

#include <memory.h>                         // For memset()

typedef struct 
{
    struct android_app *app;
    bool bActive;
} Engine;

ANativeWindow *androidNativeWindow = NULL;

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
    return 0;
}

