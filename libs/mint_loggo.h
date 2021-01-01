#ifndef MINT_LOGGO_H
#define MINT_LOGGO_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

/*
    This a single header logging library.

    The logger is highly configurable, easy to use and fast. It uses a queue for receiving messages
    on consumer threads and a hashtable for logger lookup. 

    In order to use this you must do

    #define MINT_LOGGO_IMPLEMENTATION
    #include "mint_loggo.h" 

    In only one file to actual implement the logger.

    Then in subsequent files you only have to 
    #include "mint_loggo.h"

    to use it.
*/

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(MINT_LOGGO_USE_POSIX)
    #include <pthread.h>
    #include <unistd.h>
    #define MINT_LOGGO_THREAD_TYPE pthread_t
    #define MINT_LOGGO_THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define MINT_LOGGO_THREAD_JOIN(id) pthread_join((id), (NULL))
    #define MINT_LOGGO_MUTEX_TYPE pthread_mutex_t
    #define MINT_LOGGO_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
    #define MINT_LOGGO_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
    #define MINT_LOGGO_MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
    #define MINT_LOGGO_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
    #define MINT_LOGGO_COND_TYPE pthread_cond_t
    #define MINT_LOGGO_COND_INIT(condition) pthread_cond_init(&(condition), NULL)
    #define MINT_LOGGO_COND_DESTROY(condition) pthread_cond_destroy(&(condition))
    #define MINT_LOGGO_COND_WAIT(condition, mutex) pthread_cond_wait(&(condition), &(mutex))
    #define MINT_LOGGO_COND_SIGNAL(condition) pthread_cond_signal(&(condition))
#elif defined(_WIN32) || defined(MINT_LOGGO_USE_WINDOWS)
    #include <io.h>
    #include <Windows.h>
    #define MINT_LOGGO_THREAD_TYPE LPDWORD
    #define MINT_LOGGO_THREAD_CREATE(id, func, param) CreateThread(NULL, 0, func, param, 0, id)
    #define MINT_LOGGO_THREAD_JOIN(id) WaitForSingleObject((id), INFINITE)
    #define MINT_LOGGO_MUTEX_TYPE LPCRITICAL_SECTION
    #define MINT_LOGGO_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
    #define MINT_LOGGO_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
    #define MINT_LOGGO_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
    #define MINT_LOGGO_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
    #define MINT_LOGGO_COND_TYPE PCONDITION_VARIABLE
    #define MINT_LOGGO_COND_INIT(condition) InitializeConditionVariable((condition))
    #define MINT_LOGGO_COND_DESTROY(condition) DeleteConditionVariable((condition))
    #define MINT_LOGGO_COND_WAIT(condition, mutex) SleepConditionVariableCS((condition), (mutex), INFINITE)
    #define MINT_LOGGO_COND_SIGNAL(condition) WakeConditionVariable((condition))
#endif

#ifdef MINT__DEBUG
    #include <assert.h>
#endif

#define MINT_LOGGO_UNUSED(x) (void)(x)

// Change this to change how Loggo is compiled in
// Static would obviously make scope Loggo API methods to the 
// current translation unit which could be what you want
// Defaults to extern API.
#ifndef MINT_LOGGO_DEF
    #ifdef MINT_LOGGO_DEF_STATIC
        #define MINT_LOGGO_DEF static
    #else
        #define MINT_LOGGO_DEF extern
    #endif
#endif

// Log Levels
typedef enum {
    MINT_LOGGO_LEVEL_DEBUG,
    MINT_LOGGO_LEVEL_INFO,
    MINT_LOGGO_LEVEL_WARN,
    MINT_LOGGO_LEVEL_ERROR,
    MINT_LOGGO_LEVEL_FATAL
} Mint_Loggo_LogLevel;

typedef int (*CloseHandler)(void*);
typedef int (*WriteHandler)(char*, void*);
typedef int (*FlushHandler)(void*);

typedef struct {
    void* handle;
    CloseHandler close_handler;
    WriteHandler write_handler;
    FlushHandler flush_handler;
} Mint_Loggo_LogHandler;

// The user controls the format
typedef struct {
    Mint_Loggo_LogLevel level;
    uint32_t queue_capacity;
    bool colors;
    bool flush;
    char* time_format;
    char* linesep;
    char* linebeg;
} Mint_Loggo_LogFormat;


#ifdef __cplusplus
extern "C" {
#endif


// API

/* 
 * Init Logger in its own thread.
 * If user_format is NULL then the defaults are used
 * If user_handler is NULL then buffered stdout is used for logging
 * name cannot be NULL
 * Returns logger id on success or -1 for Failure
 */
MINT_LOGGO_DEF int32_t Mint_Loggo_CreateLogger(const char* name, Mint_Loggo_LogFormat* user_format, Mint_Loggo_LogHandler* user_handler);


/* 
 * Delete logger waiting for all of its messages,
 * This will also clean up the resources if its the last logger so there is no need to call DeleteLoggers
 */
MINT_LOGGO_DEF void Mint_Loggo_DeleteLogger(const char* name);


/* 
 * Stop Logger threads and clean up handles.
 * This is idempotent so it can be called multiple times
 */
 MINT_LOGGO_DEF void Mint_Loggo_DeleteLoggers();


/* 
 * Pass messages to the log queue, the logging thread will accept messages,
 * then use the handler methods (or defaults) to output logs
 */
MINT_LOGGO_DEF void Mint_Loggo_Log(const char* name, Mint_Loggo_LogLevel level, const char* msg);
MINT_LOGGO_DEF void Mint_Loggo_Log2(const char* name, Mint_Loggo_LogLevel level, char* msg, bool free_string);

// Loggo Handler methods

// FILE* friends
MINT_LOGGO_DEF int Mint_Loggo_StreamWrite(char* text, void* arg);
MINT_LOGGO_DEF int Mint_Loggo_StreamClose(void* arg);
MINT_LOGGO_DEF int Mint_Loggo_StreamFlush(void* arg);

// Raw Descriptor IO
MINT_LOGGO_DEF int Mint_Loggo_DescriptorWrite(char* text, void* arg);
MINT_LOGGO_DEF int Mint_Loggo_DescriptorClose(void* arg);
MINT_LOGGO_DEF int Mint_Loggo_DescriptorFlush(void* arg);

// Do nothing
MINT_LOGGO_DEF int Mint_Loggo_NullWrite(char* text, void* arg);
MINT_LOGGO_DEF int Mint_Loggo_NullClose(void* arg);
MINT_LOGGO_DEF int Mint_Loggo_NullFlush(void* arg);

#ifdef __cplusplus
}
#endif

// Convenience Macros for logging
#ifdef MINT_LOGGO_USE_HELPERS
    #define LOG_DEBUG(name, msg) Mint_Loggo_Log((name), MINT_LOGGO_LEVEL_DEBUG, (msg))
    #define LOG_INFO(name, msg) Mint_Loggo_Log((name), MINT_LOGGO_LEVEL_INFO, (msg))
    #define LOG_WARN(name, msg) Mint_Loggo_Log((name), MINT_LOGGO_LEVEL_WARN, (msg))
    #define LOG_ERROR(name, msg) Mint_Loggo_Log((name), MINT_LOGGO_LEVEL_ERROR, (msg))
    #define LOG_FATAL(name, msg) Mint_Loggo_Log((name), MINT_LOGGO_LEVEL_FATAL, (msg))

    #define LOG2_DEBUG(name, msg, free_string) Mint_Loggo_Log2((name), MINT_LOGGO_LEVEL_DEBUG, (msg), (free_string))
    #define LOG2_INFO(name, msg, free_string) Mint_Loggo_Log2((name), MINT_LOGGO_LEVEL_INFO, (msg), (free_string))
    #define LOG2_WARN(name, msg, free_string) Mint_Loggo_Log2((name), MINT_LOGGO_LEVEL_WARN, (msg), (free_string))
    #define LOG2_ERROR(name, msg, free_string) Mint_Loggo_Log2((name), MINT_LOGGO_LEVEL_ERROR, (msg), (free_string))
    #define LOG2_FATAL(name, msg, free_string) Mint_Loggo_Log2((name), MINT_LOGGO_LEVEL_FATAL, (msg), (free_string))

    #define STDOUT_STREAM_HANDLER (Mint_Loggo_LogHandler) { \
                                    .handle=stdout, \
                                    .write_handler=Mint_Loggo_StreamWrite, \
                                    .close_handler=Mint_Loggo_StreamClose, \
                                    .flush_handler=Mint_Loggo_StreamFlush \
                                }

    #define STDERR_STREAM_HANDLER (Mint_Loggo_LogHandler) { \
                                    .handle=stderr, \
                                    .write_handler=Mint_Loggo_StreamWrite, \
                                    .close_handler=Mint_Loggo_StreamClose, \
                                    .flush_handler=Mint_Loggo_StreamFlush \
                                }

    #define STDOUT_DESC_HANDLER (Mint_Loggo_LogHandler) { \
                                    .handle=(&STDOUT_FILENO), \
                                    .write_handler=Mint_Loggo_DescriptorWrite, \
                                    .close_handler=Mint_Loggo_DescriptorClose, \
                                    .flush_handler=Mint_Loggo_DescriptroFlush \
                                }

    #define STDERR_DESC_HANDLER (Mint_Loggo_LogHandler) { \
                                .handle=(&STDERR_FILENO), \
                                .write_handler=Mint_Loggo_DescriptorWrite, \
                                .close_handler=Mint_Loggo_DescriptorClose, \
                                .flush_handler=Mint_Loggo_DescriptroFlush \
                            }
#endif




// Do implementation here if you do this twice when MINT_LOGGO_DEF_STATIC is not set you will get linker
// errors from multiple definitions
#ifdef MINT_LOGGO_IMPLEMENTATION


////////////////////////////////////
// Platform and Helpers
////////////////////////////////////


#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(MINT_USE_POSIX)
    #define MINT_LOGGO_RED      "\033[31m"
    #define MINT_LOGGO_GREEN    "\033[32m"
    #define MINT_LOGGO_YELLOW   "\033[33m"
    #define MINT_LOGGO_BLUE     "\033[34m"
    #define MINT_LOGGO_MAGENTA  "\033[35m"
    #define MINT_LOGGO_CYAN     "\033[36m"
    #define MINT_LOGGO_WHITE    "\033[37m"
    #define MINT_LOGGO_RESET    "\033[0m"

    MINT_LOGGO_DEF int Mint_Loggo_DescriptorWrite(char* text, void* arg) {
        return write(*(int*)arg, text, strlen(text));
    }


    MINT_LOGGO_DEF int Mint_Loggo_DescriptorClose(void* arg) {
        return close(*(int*)arg);
    }
#elif defined(_WIN32) || defined(MINT_USE_WINDOWS)
    // TODO Fix this
    #define MINT_LOGGO_RED      ""
    #define MINT_LOGGO_GREEN    ""
    #define MINT_LOGGO_YELLOW   ""
    #define MINT_LOGGO_BLUE     ""
    #define MINT_LOGGO_MAGENTA  ""
    #define MINT_LOGGO_CYAN     ""
    #define MINT_LOGGO_WHITE    ""
    #define MINT_LOGGO_RESET    ""

    MINT_LOGGO_DEF int Mint_Loggo_DescriptorWrite(char* text, void* arg) {
        return _write(*(int*)arg, text, strlen(text));
    }

    
    MINT_LOGGO_DEF int Mint_Loggo_DescriptorClose(void* arg) {
        return _close(*(int*)arg);
    }
#endif

// Common

MINT_LOGGO_DEF int Mint_Loggo_DescriptorFlush(void* arg) {
    MINT_LOGGO_UNUSED(arg);
    return 0;
}


MINT_LOGGO_DEF int Mint_Loggo_StreamWrite(char* text, void* arg) {
    return fputs(text, (FILE*)arg);
}


MINT_LOGGO_DEF int Mint_Loggo_StreamClose(void* arg) {
   return fclose((FILE*)arg);
}


MINT_LOGGO_DEF int Mint_Loggo_StreamFlush(void* arg) {
    return fflush((FILE*)arg);
}


MINT_LOGGO_DEF int Mint_Loggo_NullWrite(char* text, void* arg) {
    MINT_LOGGO_UNUSED(arg);
    MINT_LOGGO_UNUSED(text);
    return 0;
}


MINT_LOGGO_DEF int Mint_Loggo_NullClose(void* arg) {
    MINT_LOGGO_UNUSED(arg);
    return 0;
}


MINT_LOGGO_DEF int Mint_Loggo_NullFlush(void* arg) {
    MINT_LOGGO_UNUSED(arg);
    return 0;
}



static void* Mint_Loggo_ErrorCheckedMalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because malloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


static void* Mint_Loggo_ErrorCheckedRealloc(void* original, size_t size) {
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

#define MINT_LOGGO_DEFAULT_LINE_SEP "\n"
#define MINT_LOGGO_DEFAULT_LINE_BEG ""
#define MINT_LOGGO_DEFAULT_QUEUE_SIZE 1024U 
#define MINT_LOGGO_DEFAULT_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define MINT_LOGGO_DEFAULT_HT_INITIAL_CAPACITY 128
#define MINT_LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR 0.7f

// Can be overriden by user
#define MINT_LOGGO_MALLOC Mint_Loggo_ErrorCheckedMalloc
#define MINT_LOGGO_REALLOC Mint_Loggo_ErrorCheckedRealloc
#define MINT_LOGGO_FREE free


////////////////////////////////////
// Types
////////////////////////////////////

// Messages are always created and must be freed
typedef struct {
    Mint_Loggo_LogLevel level;
    bool done;
    char* msg;
} Mint_Loggo_LogMessage;



// Circular dynamic array implementation
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t capacity;
    uint32_t size;
    Mint_Loggo_LogMessage** messages;
    MINT_LOGGO_MUTEX_TYPE queue_lock;
    MINT_LOGGO_COND_TYPE queue_not_full;
    MINT_LOGGO_COND_TYPE queue_not_empty;
} Mint_Loggo_LogQueue;


// Contains everything a logger will need
typedef struct {
    Mint_Loggo_LogFormat* format;
    Mint_Loggo_LogHandler* handler;
    Mint_Loggo_LogQueue* queue;
    int32_t id;
    MINT_LOGGO_THREAD_TYPE thread_id;
    const char* name;
    bool done;

} Mint_Loggo_Logger;

typedef struct {
    Mint_Loggo_Logger** loggers;
    int32_t size;
    int32_t capacity;
    double load_factor;
} Mint_Loggo_HashTable;


////////////////////////////////////
// Constants
////////////////////////////////////
static const int32_t MINT_LOGGO_PRIME_1 = 71U;
static const int32_t MINT_LOGGO_PRIME_2 = 197U;


////////////////////////////////////
// Global values
////////////////////////////////////
static Mint_Loggo_LogMessage MINT_LOGGO_LOGGER_TERMINATE = {.done = true};
static Mint_Loggo_Logger MINT_LOGGO_LOGGER_DELETED = {0};
static Mint_Loggo_HashTable MINT_LOGGO_LOGGER_HASH_TABLE = {0};


////////////////////////////////////
// Declarations up front (not including api)
////////////////////////////////////


// Queue
static Mint_Loggo_LogQueue* Mint_Loggo_CreateQueue(uint32_t capacity);
static void Mint_Loggo_DestroyQueue(Mint_Loggo_LogQueue* queue);
static bool Mint_Loggo_IsQueueFull(Mint_Loggo_LogQueue* queue);
static bool Mint_Loggo_IsQueueEmpty(Mint_Loggo_LogQueue* queue);
static void Mint_Loggo_Enqueue(Mint_Loggo_LogQueue* queue, Mint_Loggo_LogMessage* message);
static Mint_Loggo_LogMessage* Mint_Loggo_Dequeue(Mint_Loggo_LogQueue* queue);

// Logging
static void* Mint_Loggo_RunLogger(void* arg);
static char* Mint_Loggo_StringFromLevel(Mint_Loggo_LogLevel level);
static char* Mint_Loggo_ColorFromLevel(Mint_Loggo_LogLevel level);
static Mint_Loggo_LogMessage* Mint_Loggo_CreateLogMessage(Mint_Loggo_Logger* logger, Mint_Loggo_LogLevel level, const char* msg);
static Mint_Loggo_LogFormat* Mint_Loggo_CreateLogFormat(Mint_Loggo_LogFormat* user_format);
static Mint_Loggo_LogHandler* Mint_Loggo_CreateLogHandler(Mint_Loggo_LogHandler* user_handler);
static void Mint_Loggo_DestroyLogHandler(Mint_Loggo_LogHandler* handler);
static void Mint_Loggo_DestroyLogFormat(Mint_Loggo_LogFormat* format);
static void Mint_Loggo_CleanUpLogger(Mint_Loggo_Logger* logger);
static void Mint_Loggo_HandleLogMessage(Mint_Loggo_LogMessage* message, Mint_Loggo_LogFormat* format, Mint_Loggo_LogHandler* handler);

// Hash Table
static Mint_Loggo_Logger* Mint_Loggo_HTFindItem(const char* name);
static int32_t Mint_Loggo_HTInsertItem(const char* name, Mint_Loggo_Logger* logger);
static void Mint_Loggo_HTDeleteItem(const char* name);
static void Mint_Loggo_HTResizeTable();
static void Mint_Loggo_HTInitTable();
static void Mint_Loggo_HTDeleteTable();
static int32_t Mint_Loggo_StringHash(const char* name, const int32_t prime, const int32_t buckets);
static int32_t Mint_Loggo_LoggerStringHash(const char* name, const int32_t buckets, const int32_t attempt);



////////////////////////////////////
// Api
////////////////////////////////////


// Create logger components, start thread with the created queue
// Return -1 on error dont allocate anything, clean slate
MINT_LOGGO_DEF int32_t Mint_Loggo_CreateLogger(const char* name, Mint_Loggo_LogFormat* user_format, Mint_Loggo_LogHandler* user_handler) {
    #ifdef MINT__DEBUG
        assert(name);
    #endif

    if (!name) {
        return -1;
    }

    Mint_Loggo_HTInitTable();

    Mint_Loggo_Logger* logger = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_Logger));
    memset(logger, 0U, sizeof(*logger));

    // Fill up with info
    logger->handler = Mint_Loggo_CreateLogHandler(user_handler);

    // Clean up and return -1
    if (!logger->handler) {
        Mint_Loggo_DestroyLogHandler(logger->handler);
        memset(logger, 0U, sizeof(*logger));
        MINT_LOGGO_FREE(logger);
        logger = NULL;
        return  -1;
    }

    logger->format = Mint_Loggo_CreateLogFormat(user_format);
    logger->name = name;
    logger->queue = Mint_Loggo_CreateQueue(logger->format->queue_capacity);

    // Handle the string allocation to a logger id
    int32_t id = Mint_Loggo_HTInsertItem(name, logger);

    // We failed
    if (id == -1) {
        Mint_Loggo_DestroyLogFormat(logger->format);
        Mint_Loggo_DestroyLogHandler(logger->handler);
        Mint_Loggo_DestroyQueue(logger->queue);
        memset(logger, 0U, sizeof(*logger));
        MINT_LOGGO_FREE(logger);
        logger = NULL;
        return  id;
    }

    // Handle new stuff
    logger->id = id;

    // Spin up a thread for the loggers
    MINT_LOGGO_THREAD_CREATE(&logger->thread_id, Mint_Loggo_RunLogger, ((void*)logger));

    // Return Id to user
    return logger->id;
}


// Shutdown the loggers by iterating and setting values
MINT_LOGGO_DEF void Mint_Loggo_DeleteLoggers() {
    for (int32_t idx = 0; idx < MINT_LOGGO_LOGGER_HASH_TABLE.capacity; idx++) {
        // Delete non null items by grabbing there names from the table
        if (MINT_LOGGO_LOGGER_HASH_TABLE.loggers[idx] != NULL && MINT_LOGGO_LOGGER_HASH_TABLE.loggers[idx] != &MINT_LOGGO_LOGGER_DELETED) {
            Mint_Loggo_HTDeleteItem(MINT_LOGGO_LOGGER_HASH_TABLE.loggers[idx]->name);
        }
    }

    Mint_Loggo_HTDeleteTable();
    memset(&MINT_LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(Mint_Loggo_HashTable));
}


// Shutdown the loggers by iterating and setting values
MINT_LOGGO_DEF void Mint_Loggo_DeleteLogger(const char* name) {
    Mint_Loggo_HTDeleteItem(name);

    // Just delete the table and clear it so its inited next time
    if (MINT_LOGGO_LOGGER_HASH_TABLE.size == 0) {
        Mint_Loggo_HTDeleteTable();
        memset(&MINT_LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(Mint_Loggo_HashTable));
    }
}


// Log message with Enqueue
MINT_LOGGO_DEF void Mint_Loggo_Log(const char* name, Mint_Loggo_LogLevel level, const char* msg) {
    #ifdef MINT__DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    Mint_Loggo_Logger* logger = Mint_Loggo_HTFindItem(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        Mint_Loggo_DeleteLoggers();
        exit(EXIT_FAILURE);
    }

    Mint_Loggo_LogMessage* message = Mint_Loggo_CreateLogMessage(logger, level, msg);


    Mint_Loggo_Enqueue(logger->queue, message);
}


// Log message with Enqueue, optionally free msg if free_string is true. In case the user wants to pass
// a malloced string in
MINT_LOGGO_DEF void Mint_Loggo_Log2(const char* name, Mint_Loggo_LogLevel level, char* msg, bool free_string) {
    #ifdef MINT__DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    Mint_Loggo_Logger* logger = Mint_Loggo_HTFindItem(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        Mint_Loggo_DeleteLoggers();
        exit(EXIT_FAILURE);
    }

    Mint_Loggo_LogMessage* message = Mint_Loggo_CreateLogMessage(logger, level, msg);

    if (free_string) {
        free(msg);
    }
    
    Mint_Loggo_Enqueue(logger->queue, message);
}


// Queue

// Create the queue with sane defaults
static Mint_Loggo_LogQueue* Mint_Loggo_CreateQueue(uint32_t capacity) {
    #ifdef MINT__DEBUG
        assert(capacity > 0U);
    #endif

    Mint_Loggo_LogQueue* queue = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_LogQueue));

    // Clear out values and set actual ones
    memset(queue, 0U, sizeof(*queue));
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->size = 0U;

    // Init locks/cond
    MINT_LOGGO_MUTEX_INIT(queue->queue_lock);
    MINT_LOGGO_COND_INIT(queue->queue_not_full);
    MINT_LOGGO_COND_INIT(queue->queue_not_empty);

    // Init messsages circular buffer
    queue->messages = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_LogMessage*) * queue->capacity);
    memset(queue->messages, 0U, sizeof(Mint_Loggo_LogMessage*) * queue->capacity);
    return queue;
}


// Free messages, queue and zero out mem
static void Mint_Loggo_DestroyQueue(Mint_Loggo_LogQueue* queue) {
    #ifdef MINT__DEBUG
        assert(queue);
    #endif

    // Safer to go over all of them just in case and free shit,
    // The terminate in the thread loop should do this
    uint32_t start = queue->tail;
    uint32_t end = queue->head;
    while(start != end) {
        if(queue->messages[start]) {
            if (queue->messages[start]->msg) {
                MINT_LOGGO_FREE(queue->messages[start]->msg);
                queue->messages[start]->msg = NULL;
            }
            MINT_LOGGO_FREE(queue->messages[start]);
            queue->messages[start] = NULL;
        }
        
        // Wraparound
        start = (start + 1) % queue->capacity;
    }

    // Clean up threading stuff
    MINT_LOGGO_MUTEX_DESTROY((queue->queue_lock));
    MINT_LOGGO_COND_DESTROY((queue->queue_not_empty));
    MINT_LOGGO_COND_DESTROY((queue->queue_not_full));

    // Free messages that the queue owns
    if (queue->messages) {
        MINT_LOGGO_FREE(queue->messages);
        queue->messages = NULL;
    }

    // Clean up last bits of memory
    memset(queue, 0U, sizeof(*queue));
    MINT_LOGGO_FREE(queue);
}


// If head + 1 == tail
static bool Mint_Loggo_IsQueueFull(Mint_Loggo_LogQueue* queue) {
    return queue->head + 1 == queue->tail;
}

 
// If head == tail
static bool Mint_Loggo_IsQueueEmpty(Mint_Loggo_LogQueue* queue) {
    return queue->head == queue->tail;
}


// Wait for the queue to not be full
// Add message
// Signal that its not empty anymore
static void Mint_Loggo_Enqueue(Mint_Loggo_LogQueue* queue, Mint_Loggo_LogMessage* message) {
    MINT_LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef MINT__DEBUG
        assert(queue);
        assert(message);
    #endif
    
    // Just dont queue if full
    while (Mint_Loggo_IsQueueFull(queue)) {
        MINT_LOGGO_COND_WAIT(queue->queue_not_full, queue->queue_lock);
    }
    
    // Add message and advance queue
    queue->messages[queue->head] = message;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size++;

    // Let the thread know it has a message
    MINT_LOGGO_COND_SIGNAL(queue->queue_not_empty);
    MINT_LOGGO_MUTEX_UNLOCK(queue->queue_lock);
}


// If the queue is empty just wait until we get the okay from Enqueue
// Also let enqueue know we are not full because we took a message
static Mint_Loggo_LogMessage* Mint_Loggo_Dequeue(Mint_Loggo_LogQueue* queue) {
    MINT_LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef MINT__DEBUG
        assert(queue);
    #endif

    while (Mint_Loggo_IsQueueEmpty(queue)) {
        MINT_LOGGO_COND_WAIT(queue->queue_not_empty, queue->queue_lock);
    }

    Mint_Loggo_LogMessage* message = queue->messages[queue->tail];
    queue->messages[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size--;

    MINT_LOGGO_COND_SIGNAL(queue->queue_not_full);
    MINT_LOGGO_MUTEX_UNLOCK(queue->queue_lock);
    return message; 
}


// Logging


// Stringify Level
static char* Mint_Loggo_StringFromLevel(Mint_Loggo_LogLevel level) {
    char* result = NULL;
    switch(level) {
        case MINT_LOGGO_LEVEL_DEBUG:
            result = "DEBUG";
            break;
        case MINT_LOGGO_LEVEL_INFO:
            result = "INFO";
            break;
        case MINT_LOGGO_LEVEL_WARN:
            result = "WARN";
            break;
        case MINT_LOGGO_LEVEL_ERROR:
            result = "ERROR";
            break;
        case MINT_LOGGO_LEVEL_FATAL:
            result = "FATAL";
            break;
        default:
            result = "UNKNOWN";
            break;
    }

    return result;
}


// Colorify Level
static char* Mint_Loggo_ColorFromLevel(Mint_Loggo_LogLevel level) {
    char* result = NULL;
    switch(level) {
        case MINT_LOGGO_LEVEL_DEBUG:
            result = MINT_LOGGO_MAGENTA;
            break;
        case MINT_LOGGO_LEVEL_INFO:
            result = MINT_LOGGO_GREEN;
            break;
        case MINT_LOGGO_LEVEL_WARN:
            result = MINT_LOGGO_YELLOW;
            break;
        case MINT_LOGGO_LEVEL_ERROR:
            result = MINT_LOGGO_CYAN;
            break;
        case MINT_LOGGO_LEVEL_FATAL:
            result = MINT_LOGGO_RED;
            break;
        default:
            result = MINT_LOGGO_WHITE;
            break;
    }

    return result;
}


static Mint_Loggo_LogFormat* Mint_Loggo_CreateLogFormat(Mint_Loggo_LogFormat* user_format) {
    #ifdef MINT__DEBUG
        assert(user_format);
    #endif

    // Create User handler
    Mint_Loggo_LogFormat* log_format = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_LogFormat));
    memset(log_format, 0U, sizeof(*log_format));

    // Copy user format if it exists
    if (user_format) { 
        memcpy(log_format, user_format, sizeof(*user_format));
    }

    // Set defaults
    // Level defaults to LOG_DEBUG
    // colors defaults to 0 (false)
    // flush defaults to 0 (false)

    if (!log_format->linesep) log_format->linesep = MINT_LOGGO_DEFAULT_LINE_SEP;
    if (log_format->queue_capacity == 0) log_format->queue_capacity = MINT_LOGGO_DEFAULT_QUEUE_SIZE;
    if (!log_format->time_format) log_format->time_format = MINT_LOGGO_DEFAULT_TIME_FORMAT;
    if (!log_format->linebeg) log_format->linebeg = MINT_LOGGO_DEFAULT_LINE_BEG;
    return log_format;
}


static Mint_Loggo_LogHandler* Mint_Loggo_CreateLogHandler(Mint_Loggo_LogHandler* user_handler) {

    // Create User handler
    Mint_Loggo_LogHandler* log_handler = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_LogHandler));
    memset(log_handler, 0U, sizeof(*log_handler));

    // Copy user format if it exists
    if (user_handler) { 
        memcpy(log_handler, user_handler, sizeof(*user_handler));

        // You need the handle
        if (!log_handler->handle) {
            Mint_Loggo_DestroyLogHandler(log_handler);
            return NULL;
        }

        // Set NULL handlers if they are not there
        if(!log_handler->write_handler) log_handler->write_handler = Mint_Loggo_NullWrite;
        if(!log_handler->flush_handler) log_handler->flush_handler = Mint_Loggo_NullFlush;
        if(!log_handler->close_handler) log_handler->close_handler = Mint_Loggo_NullClose;

    } else {
        // Defaults
        log_handler->handle = stdout;
        log_handler->write_handler = Mint_Loggo_StreamWrite;
        log_handler->close_handler = Mint_Loggo_StreamClose;
        log_handler->flush_handler = Mint_Loggo_StreamFlush;
    }

    return log_handler;  
}


static void Mint_Loggo_DestroyLogHandler(Mint_Loggo_LogHandler* handler) {
    #ifdef MINT__DEBUG
        assert(handler);
    #endif

    memset(handler, 0U, sizeof(*handler));
    MINT_LOGGO_FREE(handler);  
}


static void Mint_Loggo_DestroyLogFormat(Mint_Loggo_LogFormat* format) {
    #ifdef MINT__DEBUG
        assert(format);
    #endif

    memset(format, 0U, sizeof(*format));
    MINT_LOGGO_FREE(format);
}


// Actual ouptut of message and cleanup
static void Mint_Loggo_HandleLogMessage(Mint_Loggo_LogMessage* message, Mint_Loggo_LogFormat* format, Mint_Loggo_LogHandler* handler) {
    #ifdef MINT__DEBUG
        assert(message);
        assert(format);
        assert(handler);
    #endif

    if (message->level >= format->level) {
        if (format->colors) {
            handler->write_handler(Mint_Loggo_ColorFromLevel(message->level), handler->handle);
        }

        // Write actual output
        handler->write_handler(format->linebeg, handler->handle);
        handler->write_handler(" ", handler->handle);
        handler->write_handler(message->msg, handler->handle);
        handler->write_handler(format->linesep, handler->handle);

        // Reset colors
        if (format->colors) {
            handler->write_handler(MINT_LOGGO_RESET, handler->handle);
        }

        // Flush if needed
        if (format->flush) {
            handler->flush_handler(handler->handle);
        }
    }

    // Clean up message
    MINT_LOGGO_FREE(message->msg);
    message->msg = NULL;

    // Clean up message
    MINT_LOGGO_FREE(message);
}


// Thread spawned handler of messages
static void* Mint_Loggo_RunLogger(void* arg) {
    #ifdef MINT__DEBUG
        assert(arg);
    #endif
    Mint_Loggo_Logger* logger = arg;
    #ifdef MINT__DEBUG
        assert(logger->queue);
    #endif

    // For some reason you have to grab the read lock and read all that you can in a loop
    // Or else the condition is never signaled and you wait
    while (!logger->done) {
        Mint_Loggo_LogMessage* message = Mint_Loggo_Dequeue(logger->queue);

        #ifdef MINT__DEBUG
            assert(logger);
            assert(message);
        #endif

        if (message) {
            // Done at this point
            if (message->done) {
                logger->done = true;
                continue;
            }

            // Log the messages, then free them
            Mint_Loggo_HandleLogMessage(message, logger->format, logger->handler);
        }
    }

    return EXIT_SUCCESS;
}


// Create a nice formatted log message
static Mint_Loggo_LogMessage* Mint_Loggo_CreateLogMessage(Mint_Loggo_Logger* logger, Mint_Loggo_LogLevel level, const char* msg) {
    // Misc
    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;

    // Level String
    const char* level_string = Mint_Loggo_StringFromLevel(level);
    uint32_t level_size = strlen(level_string);

    // Get time
    char time_buffer[128U];
    time_t current_time = time(NULL);
    time_buffer[strftime(time_buffer, sizeof(time_buffer), logger->format->time_format, localtime(&current_time))] = '\0';

    // Create LogMessage
    Mint_Loggo_LogMessage* message = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_LogMessage));
    memset(message, 0U, sizeof(*message));
    
    // Insert formatted message inside of LogMessage
    formatted_msg = MINT_LOGGO_MALLOC(strlen(msg) + strlen(time_buffer) + level_size + padding + 1U);
    sprintf(formatted_msg, "[%s] %s %s", time_buffer, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    return message;
}


// Free all the handles
static void Mint_Loggo_CleanUpLogger(Mint_Loggo_Logger* logger) {

    // Queue up final message and wait for logger to close
    Mint_Loggo_Enqueue(logger->queue, &MINT_LOGGO_LOGGER_TERMINATE);
    MINT_LOGGO_THREAD_JOIN(logger->thread_id);

    // Free handles
    Mint_Loggo_DestroyLogHandler(logger->handler);
    logger->handler = NULL;

    Mint_Loggo_DestroyLogFormat(logger->format);
    logger->format = NULL;

    Mint_Loggo_DestroyQueue(logger->queue);
    logger->queue = NULL;

    MINT_LOGGO_FREE(logger);
}


// Logger hash table


// Try to find an item returning NULL if not found
static Mint_Loggo_Logger* Mint_Loggo_HTFindItem(const char* name) {
    int32_t hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    Mint_Loggo_Logger* current_logger = MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL) {
        if (current_logger != &MINT_LOGGO_LOGGER_DELETED) {
            if (strcmp(name, current_logger->name) == 0) {
                return current_logger;
            }
        }
        hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }

    return NULL;
}


// Try to insert an item, resize if needed
static int32_t Mint_Loggo_HTInsertItem(const char* name, Mint_Loggo_Logger* logger) {
    // Try a resize
    Mint_Loggo_HTResizeTable();
    int32_t hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    Mint_Loggo_Logger* current_logger = MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL && current_logger != &MINT_LOGGO_LOGGER_DELETED) {
        hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        // If the item exists update it
        if (current_logger) {
            if (strcmp(name, current_logger->name) == 0) {
                Mint_Loggo_CleanUpLogger(current_logger);
                MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
                return logger->id;
            }
        }

        attempt++;

    }

    MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
    MINT_LOGGO_LOGGER_HASH_TABLE.size++;
    logger->id = hash;
    return logger->id;
}


static void Mint_Loggo_HTDeleteItem( const char* name) {
    int32_t hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    Mint_Loggo_Logger* current_logger = MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    int32_t attempt = 1;
    while(current_logger != NULL) {
        if (current_logger != &MINT_LOGGO_LOGGER_DELETED && current_logger != NULL) {
            if (strcmp(current_logger->name, name) == 0) {
                Mint_Loggo_CleanUpLogger(current_logger);
                MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash] = &MINT_LOGGO_LOGGER_DELETED;
            }
        }
        hash = Mint_Loggo_LoggerStringHash(name, MINT_LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger =  MINT_LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }
    MINT_LOGGO_LOGGER_HASH_TABLE.size--; 
}


// Just realloc space up
static void Mint_Loggo_HTResizeTable() {
    int32_t load = (MINT_LOGGO_LOGGER_HASH_TABLE.size * 100) / MINT_LOGGO_LOGGER_HASH_TABLE.capacity;
    if (load > (MINT_LOGGO_LOGGER_HASH_TABLE.load_factor * 100)) {
        MINT_LOGGO_LOGGER_HASH_TABLE.capacity *= 2;

        // No need to realloc a new table
        // Just realloc memory chunk
        MINT_LOGGO_LOGGER_HASH_TABLE.loggers = MINT_LOGGO_REALLOC(MINT_LOGGO_LOGGER_HASH_TABLE.loggers, sizeof(Mint_Loggo_Logger*) * MINT_LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Delete table and set defaults
static void Mint_Loggo_HTDeleteTable() {
    if (MINT_LOGGO_LOGGER_HASH_TABLE.loggers) {
        MINT_LOGGO_FREE(MINT_LOGGO_LOGGER_HASH_TABLE.loggers);
        MINT_LOGGO_LOGGER_HASH_TABLE.loggers = NULL;
    }

    MINT_LOGGO_LOGGER_HASH_TABLE.capacity = MINT_LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
    MINT_LOGGO_LOGGER_HASH_TABLE.size = 0;
    MINT_LOGGO_LOGGER_HASH_TABLE.load_factor = MINT_LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
}


// Init whats needed
static void Mint_Loggo_HTInitTable() {
    if (!MINT_LOGGO_LOGGER_HASH_TABLE.loggers) {
        MINT_LOGGO_LOGGER_HASH_TABLE.capacity = MINT_LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
        MINT_LOGGO_LOGGER_HASH_TABLE.size = 0;
        MINT_LOGGO_LOGGER_HASH_TABLE.load_factor = MINT_LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
        MINT_LOGGO_LOGGER_HASH_TABLE.loggers = MINT_LOGGO_MALLOC(sizeof(Mint_Loggo_Logger*) * MINT_LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Simple hash
static int32_t Mint_Loggo_StringHash(const char* name, const int32_t prime, const int32_t buckets) {
    int32_t hash = 0;
    const int32_t len = strlen(name);
    for (int32_t i = 0; i < len; i++) {
        hash += (int32_t)pow(prime, len - (i+1)) * name[i];
        hash = hash % buckets;
    }
    return hash; 
}


// Double hash
static int32_t Mint_Loggo_LoggerStringHash(const char* name, const int32_t buckets, const int32_t attempt) {
    const int32_t hasha = Mint_Loggo_StringHash(name, MINT_LOGGO_PRIME_1, buckets);
    const int32_t hashb = Mint_Loggo_StringHash(name, MINT_LOGGO_PRIME_2, buckets);
    return (hasha + (attempt * (hashb + 1))) % buckets;
}


#endif // MINT_LOGGO_IMPLEMENTATION
#undef MINT_LOGGO_IMPLEMENTATION

#endif // MINT_LOGGO_H
