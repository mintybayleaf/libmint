#ifndef MINT_EZEMU_H
#define MINT_EZEMU_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <SDL/SDL2.h>

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(MINT_USE_POSIX)
    // TODO Fill in
#elif defined(_WIN32) || defined(MINT_USE_WINDOWS)
    // TODO Fill in
#endif

#ifdef MINT__DEBUG
    #include <assert.h>
#endif


// Change this to change how Ezgui is compiled in
// Static would obviously make scope Ezgui API methods to the 
// current translation unit which could be what you want
// Defaults to extern API so define EZEMU_IMPLEMENTATION in the file you want
// to provide definitions in
#ifndef MINTDEF
    #ifdef MINT_DEF_STATIC
        #define MINTDEF static
    #else
        #define MINTDEF extern
    #endif
#endif


////////////////////////////////////
// Api
////////////////////////////////////


typedef enum {
    EZEMU_TEXTURE_DISPLAY,
    EZEMU_TEXT_DISPLAY
} EzEmu_DisplayType;


// For playing audio
typedef struct {
    const char* audio_device_name;
    uint32_t samples;
    uint32_t channels;
    SDL_AudioFormat format;
    SDL_AudioCallback user_callback;
    void* user_audio_buffer;
    uint32_t delay_ms;
} EzEmu_AudioDevice;


// User EzEmu format
typedef struct {
    const char* title;
    uint32_t width;
    uint32_t height;
    uint32_t display_scale;
    uint32_t xpos;
    uint32_t ypos;
    uint32_t delay_ms;
    SDL_Color background_color;
    SDL_Color foreground_color;
    bool visible;
    bool vsync;
    uint32_t target_fps;
    // EzEmu_AudioDevice audio;
} EzEmu_Format;

// Base renderer display type
typedef struct {
    EzEmu_Format format;
    SDL_Window* window;
    SDL_Renderer* renderer;
    uint32_t current_fps;
    void (**event_callbacks)(uint32_t event_type, void* handle);
} EzEmu_HardwareAccelDisplay;


// For rendering textures to the screen
typedef struct {
    EzEmu_HardwareAccelDisplay display;
    SDL_Texture* texture;
    uint32_t texture_format;
    uint32_t* video_buffer;
    uint32_t video_pitch;
} EzEmu_TextureDisplay;


// For displaying lines of text on the screen
typedef struct {
    EzEmu_HardwareAccelDisplay texture_display;
    TTF_Font* font;
    SDL_Surface* text;
    uint32_t padding;
    bool fill_and_wrap;
} EzEmu_TextDisplay;


// For setting up display types
typedef struct {
    EzEmu_DisplayType type;
    union {
        EzEmu_TextureDisplay* texture;
        EzEmu_TextDisplay* text;
    };
} EzEmu_Display;


/*
 * Initialize the EzEmuSystem with the specified error output stream
 * EzEmu can close this when done if close is true
 */ 
MINTDEF void EzEmu_InitSystem(FILE* error_stream, bool close);


/*
 * This will initialize a format with defaults.
 * For custom formats the user should mess with the structure themselves...
 */ 
MINTDEF void EzEmu_InitFormat(EzEmu_Format* format, const char* title);


/*
 * This will create a texture display
 * user_format is copied over byte by byte
 */
MINTDEF EzEmu_Display* EzEmu_CreateTextureDisplay(EzEmu_Format* user_format, uint32_t* video_buffer);


/*
 * This will create a text display and register it with the system
 * user_format is copied over byte by byte
 */ 
MINTDEF EzEmu_Display* EzEmu_CreateTextDisplay(EzEmu_Format* user_format, uint32_t padding, bool wrap_and_fill);


/*
 * Register event callbacks for the display
 */
MINTDEF void EzEmu_RegisterEventCallback(EzEmu_Display* display, uint32_t event_type, void(*event_callback)(uint32_t event_type, void* handle));


/*
 * This will launch a thread for the display passed into it
 */ 
MINTDEF void EzEmu_LaunchThread(EzEmu_Display* display);


/*
 * This will update the current window/display, useful for your own loops.
 * Use this if you dont want to use threading.
 */ 
MINTDEF void EzEmu_UpdateDisplay(EzEmu_Display* display);



/*
 * This destroy all the resources and wait for the threads to close if any were launched
 */ 
MINTDEF void EzEmu_ShutdownSystem();


#ifdef EZEMU_IMPLEMENTATION

////////////////////////////////////
// Platform and Helpers
////////////////////////////////////

#define EZEMU_UNUSED(x) (void)(x)

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(MINT_USE_POSIX)
    #define EZEMU_THREAD_TYPE pthread_t
    #define EZEMU_THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define EZEMU_THREAD_JOIN(id) pthread_join((id), (NULL))
    #define EZEMU_MUTEX_TYPE pthread_mutex_t
    #define EZEMU_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
    #define EZEMU_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
    #define EZEMU_MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
    #define EZEMU_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
    #define EZEMU_COND_TYPE pthread_cond_t
    #define EZEMU_COND_INIT(condition) pthread_cond_init(&(condition), NULL)
    #define EZEMU_COND_DESTROY(condition) pthread_cond_destroy(&(condition))
    #define EZEMU_COND_WAIT(condition, mutex) pthread_cond_wait(&(condition), &(mutex))
    #define EZEMU_COND_SIGNAL(condition) pthread_cond_signal(&(condition))

#elif defined(_WIN32) || defined(MINT_USE_WINDOWS)
    #define EZEMU_THREAD_TYPE LPDWORD
    #define EZEMU_THREAD_CREATE(id, func, param) CreateThread(NULL, 0, func, param, 0, id)
    #define EZEMU_THREAD_JOIN(id) WaitForSingleObject((id), INFINITE)
    #define EZEMU_MUTEX_TYPE LPCRITICAL_SECTION
    #define EZEMU_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
    #define EZEMU_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
    #define EZEMU_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
    #define EZEMU_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
    #define EZEMU_COND_TYPE PCONDITION_VARIABLE
    #define EZEMU_COND_INIT(condition) InitializeConditionVariable((condition))
    #define EZEMU_COND_DESTROY(condition) DeleteConditionVariable((condition))
    #define EZEMU_COND_WAIT(condition, mutex) SleepConditionVariableCS((condition), (mutex), INFINITE)
    #define EZEMU_COND_SIGNAL(condition) WakeConditionVariable((condition))

#endif

static void* EzEmu_ErrorCheckedMalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because malloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


static void* EzEmu_ErrorCheckedRealloc(void* original, size_t size) {
    void* ptr = realloc(original, size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because realloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


////////////////////////////////////
// Defaults
////////////////////////////////////

// Can be overriden by user
#define EZEMU_MALLOC EzEmu_ErrorCheckedMalloc
#define EZEMU_REALLOC EzEmu_ErrorCheckedRealloc
#define EZEMU_FREE free


#endif // EZEMU_IMPLEMENTATION

#endif // MINT_EZEMU_H