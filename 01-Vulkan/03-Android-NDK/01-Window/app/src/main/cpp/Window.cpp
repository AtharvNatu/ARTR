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

