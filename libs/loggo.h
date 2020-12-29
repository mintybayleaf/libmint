#ifndef LOGGO_H
#define LOGGO_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(LOGGO_USE_POSIX)
    #include <pthread.h>
    #include <unistd.h>
#elif defined(_WIN32) || defined(LOGGO_USE_WINDOWS)
    #include <io.h>
    #include <Windows.h>
#endif

#ifdef MINT__DEBUG
    #include <assert.h>
#endif


// Change this to change how Loggo is compiled in
// Static would obviously make scope Loggo API methods to the 
// current translation unit which could be what you want
// Defaults to extern API so define LOGGO_IMPLEMENTATION in the file you want
// to provide definitions in
#ifndef LOGGODEF
    #ifdef LOGGO_DEF_STATIC
        #define LOGGODEF static
    #else
        #define LOGGODEF extern
    #endif
#endif

// Log Levels
typedef enum {
    LOGGO_LEVEL_DEBUG,
    LOGGO_LEVEL_INFO,
    LOGGO_LEVEL_WARN,
    LOGGO_LEVEL_ERROR,
    LOGGO_LEVEL_FATAL
} Loggo_LogLevel;

typedef int (*CloseHandler)(void*);
typedef int (*WriteHandler)(char*, void*);
typedef int (*FlushHandler)(void*);

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* handle;
    CloseHandler close_handler;
    WriteHandler write_handler;
    FlushHandler flush_handler;
} Loggo_LogHandler;

// The user controls the format
typedef struct {
    Loggo_LogLevel level;
    uint32_t queue_capacity;
    bool colors;
    bool flush;
    char* time_format;
    char* linesep;
    char* linebeg;
} Loggo_LogFormat;


// API

/* 
 * Init Logger in its own thread.
 * If user_format is NULL then the defaults are used
 * If user_handler is NULL then buffered stdout is used for logging
 * name cannot be NULL
 * Returns logger id on success or -1 for Failure
 */
LOGGODEF int32_t Loggo_CreateLogger(const char* name, Loggo_LogFormat* user_format, Loggo_LogHandler* user_handler);


/* 
 * Delete logger waiting for all of its messages,
 * This will also clean up the resources if its the last logger so there is no need to call DeleteLoggers
 */
LOGGODEF void Loggo_DeleteLogger(const char* name);


/* 
 * Stop Logger threads and clean up handles.
 * This is idempotent so it can be called multiple times
 */
 LOGGODEF void Loggo_DeleteLoggers();


/* 
 * Pass messages to the log queue, the logging thread will accept messages,
 * then use the handler methods (or defaults) to output logs
 */
LOGGODEF void Loggo_Log(const char* name, Loggo_LogLevel level, const char* msg);
LOGGODEF void Loggo_Log2(const char* name, Loggo_LogLevel level, char* msg, bool free_string);

// Loggo Handler methods

// FILE* friends
LOGGODEF int Loggo_StreamWrite(char* text, void* arg);
LOGGODEF int Loggo_StreamClose(void* arg);
LOGGODEF int Loggo_StreamFlush(void* arg);

// Raw Descriptor IO
LOGGODEF int Loggo_DescriptorWrite(char* text, void* arg);
LOGGODEF int Loggo_DescriptorClose(void* arg);
LOGGODEF int Loggo_DescriptorFlush(void* arg);

// Do nothing
LOGGODEF int Loggo_NullWrite(char* text, void* arg);
LOGGODEF int Loggo_NullClose(void* arg);
LOGGODEF int Loggo_NullFlush(void* arg);

#ifdef __cplusplus
}
#endif

// Convenience Macros for logging
#ifdef LOGGO_USE_HELPERS
    #define LOG_DEBUG(name, msg) Loggo_Log((name), LOGGO_LEVEL_DEBUG, (msg))
    #define LOG_INFO(name, msg) Loggo_Log((name), LOGGO_LEVEL_INFO, (msg))
    #define LOG_WARN(name, msg) Loggo_Log((name), LOGGO_LEVEL_WARN, (msg))
    #define LOG_ERROR(name, msg) Loggo_Log((name), LOGGO_LEVEL_ERROR, (msg))
    #define LOG_FATAL(name, msg) Loggo_Log((name), LOGGO_LEVEL_FATAL, (msg))

    #define LOG2_DEBUG(name, msg, free_string) Loggo_Log2((name), LOGGO_LEVEL_DEBUG, (msg), (free_string))
    #define LOG2_INFO(name, msg, free_string) Loggo_Log2((name), LOGGO_LEVEL_INFO, (msg), (free_string))
    #define LOG2_WARN(name, msg, free_string) Loggo_Log2((name), LOGGO_LEVEL_WARN, (msg), (free_string))
    #define LOG2_ERROR(name, msg, free_string) Loggo_Log2((name), LOGGO_LEVEL_ERROR, (msg), (free_string))
    #define LOG2_FATAL(name, msg, free_string) Loggo_Log2((name), LOGGO_LEVEL_FATAL, (msg), (free_string))

    #define STDOUT_STREAM_HANDLER (Loggo_LogHandler) { \
                                    .handle=stdout, \
                                    .write_handler=Loggo_StreamWrite, \
                                    .close_handler=Loggo_StreamClose, \
                                    .flush_handler=Loggo_StreamFlush \
                                }

    #define STDERR_STREAM_HANDLER (Loggo_LogHandler) { \
                                    .handle=stderr, \
                                    .write_handler=Loggo_StreamWrite, \
                                    .close_handler=Loggo_StreamClose, \
                                    .flush_handler=Loggo_StreamFlush \
                                }

    #define STDOUT_DESC_HANDLER (Loggo_LogHandler) { \
                                    .handle=(&STDOUT_FILENO), \
                                    .write_handler=Loggo_DescriptorWrite, \
                                    .close_handler=Loggo_DescriptorClose, \
                                    .flush_handler=Loggo_DescriptroFlush \
                                }

    #define STDERR_DESC_HANDLER (Loggo_LogHandler) { \
                                .handle=(&STDERR_FILENO), \
                                .write_handler=Loggo_DescriptorWrite, \
                                .close_handler=Loggo_DescriptorClose, \
                                .flush_handler=Loggo_DescriptroFlush \
                            }
#endif

// Do implementation here if you do this twice when LOGGO_DEF_STATIC is not set you will get linker
// errors from multiple definitions
#ifdef LOGGO_IMPLEMENTATION

////////////////////////////////////
// Platform and Helpers
////////////////////////////////////

#define LOGGO_UNUSED(x) (void)(x)

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(LOGGO_USE_POSIX)
    #define LOGGO_THREAD_TYPE pthread_t
    #define LOGGO_THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define LOGGO_THREAD_JOIN(id) pthread_join((id), (NULL))
    #define LOGGO_MUTEX_TYPE pthread_mutex_t
    #define LOGGO_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
    #define LOGGO_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
    #define LOGGO_MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
    #define LOGGO_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
    #define LOGGO_COND_TYPE pthread_cond_t
    #define LOGGO_COND_INIT(condition) pthread_cond_init(&(condition), NULL)
    #define LOGGO_COND_DESTROY(condition) pthread_cond_destroy(&(condition))
    #define LOGGO_COND_WAIT(condition, mutex) pthread_cond_wait(&(condition), &(mutex))
    #define LOGGO_COND_SIGNAL(condition) pthread_cond_signal(&(condition))
    #define LOGGO_RED      "\x1B[31m"
    #define LOGGO_GREEN    "\x1B[32m"
    #define LOGGO_YELLOW   "\x1B[33m"
    #define LOGGO_BLUE     "\x1B[34m"
    #define LOGGO_MAGENTA  "\x1B[35m"
    #define LOGGO_CYAN     "\x1B[36m"
    #define LOGGO_WHITE    "\x1B[37m"
    #define LOGGO_RESET    "\033[0m"

    LOGGODEF int Loggo_DescriptorWrite(char* text, void* arg) {
        return write(*(int*)arg, text, strlen(text));
    }


    LOGGODEF int Loggo_DescriptorClose(void* arg) {
        return close(*(int*)arg);
    }
#elif defined(_WIN32) || defined(LOGGO_USE_WINDOWS)
    #define LOGGO_THREAD_TYPE LPDWORD
    #define LOGGO_THREAD_CREATE(id, func, param) CreateThread(NULL, 0, func, param, 0, id)
    #define LOGGO_THREAD_JOIN(id) WaitForSingleObject((id), INFINITE)
    #define LOGGO_MUTEX_TYPE LPCRITICAL_SECTION
    #define LOGGO_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
    #define LOGGO_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
    #define LOGGO_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
    #define LOGGO_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
    #define LOGGO_COND_TYPE PCONDITION_VARIABLE
    #define LOGGO_COND_INIT(condition) InitializeConditionVariable((condition))
    #define LOGGO_COND_DESTROY(condition) DeleteConditionVariable((condition))
    #define LOGGO_COND_WAIT(condition, mutex) SleepConditionVariableCS((condition), (mutex), INFINITE)
    #define LOGGO_COND_SIGNAL(condition) WakeConditionVariable((condition))
    // TODO Fix this
    #define LOGGO_RED      ""
    #define LOGGO_GREEN    ""
    #define LOGGO_YELLOW   ""
    #define LOGGO_BLUE     ""
    #define LOGGO_MAGENTA  ""
    #define LOGGO_CYAN     ""
    #define LOGGO_WHITE    ""
    #define LOGGO_RESET    ""

    LOGGODEF int Loggo_DescriptorWrite(char* text, void* arg) {
        return _write(*(int*)arg, text, strlen(text));
    }

    
    LOGGODEF int Loggo_DescriptorClose(void* arg) {
        return _close(*(int*)arg);
    }
#endif

// Common

LOGGODEF int Loggo_DescriptorFlush(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}


LOGGODEF int Loggo_StreamWrite(char* text, void* arg) {
    return fputs(text, (FILE*)arg);
}


LOGGODEF int Loggo_StreamClose(void* arg) {
   return fclose((FILE*)arg);
}


LOGGODEF int Loggo_StreamFlush(void* arg) {
    return fflush((FILE*)arg);
}


LOGGODEF int Loggo_NullWrite(char* text, void* arg) {
    LOGGO_UNUSED(arg);
    LOGGO_UNUSED(text);
    return 0;
}


LOGGODEF int Loggo_NullClose(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}


LOGGODEF int Loggo_NullFlush(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}



static void* Loggo_ErrorCheckedMalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because malloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


static void* Loggo_ErrorCheckedRealloc(void* original, size_t size) {
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

#define LOGGO_DEFAULT_LINE_SEP "\n"
#define LOGGO_DEFAULT_LINE_BEG ""
#define LOGGO_DEFAULT_QUEUE_SIZE 1024U 
#define LOGGO_DEFAULT_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define LOGGO_DEFAULT_HT_INITIAL_CAPACITY 128
#define LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR 0.7f

// Can be overriden by user
#define LOGGO_MALLOC Loggo_ErrorCheckedMalloc
#define LOGGO_REALLOC Loggo_ErrorCheckedRealloc
#define LOGGO_FREE free


////////////////////////////////////
// Types
////////////////////////////////////

// Messages are always created and must be freed
typedef struct {
    Loggo_LogLevel level;
    bool done;
    char* msg;
} Loggo_LogMessage;



// Circular dynamic array implementation
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t capacity;
    uint32_t size;
    Loggo_LogMessage** messages;
    LOGGO_MUTEX_TYPE queue_lock;
    LOGGO_COND_TYPE queue_not_full;
    LOGGO_COND_TYPE queue_not_empty;
} Loggo_LogQueue;


// Contains everything a logger will need
typedef struct {
    Loggo_LogFormat* format;
    Loggo_LogHandler* handler;
    Loggo_LogQueue* queue;
    int32_t id;
    LOGGO_THREAD_TYPE thread_id;
    const char* name;
    bool done;

} Loggo_Logger;

typedef struct {
    Loggo_Logger** loggers;
    int32_t size;
    int32_t capacity;
    double load_factor;
} Loggo_HashTable;


////////////////////////////////////
// Constants
////////////////////////////////////
static const int32_t LOGGO_PRIME_1 = 71U;
static const int32_t LOGGO_PRIME_2 = 197U;


////////////////////////////////////
// Global values
////////////////////////////////////
static Loggo_LogMessage LOGGO_LOGGER_TERMINATE = {.done = true};
static Loggo_Logger LOGGO_LOGGER_DELETED = {0};
static Loggo_HashTable LOGGO_LOGGER_HASH_TABLE = {0};


////////////////////////////////////
// Declarations up front (not including api)
////////////////////////////////////


// Queue
static Loggo_LogQueue* Loggo_CreateQueue(uint32_t capacity);
static void Loggo_DestroyQueue(Loggo_LogQueue* queue);
static bool Loggo_IsQueueFull(Loggo_LogQueue* queue);
static bool Loggo_IsQueueEmpty(Loggo_LogQueue* queue);
static void Loggo_Enqueue(Loggo_LogQueue* queue, Loggo_LogMessage* message);
static Loggo_LogMessage* Loggo_Dequeue(Loggo_LogQueue* queue);

// Logging
static void* Loggo_RunLogger(void* arg);
static char* Loggo_StringFromLevel(Loggo_LogLevel level);
static char* Loggo_ColorFromLevel(Loggo_LogLevel level);
static Loggo_LogMessage* Loggo_CreateLogMessage(Loggo_Logger* logger, Loggo_LogLevel level, const char* msg);
static Loggo_LogFormat* Loggo_CreateLogFormat(Loggo_LogFormat* user_format);
static Loggo_LogHandler* Loggo_CreateLogHandler(Loggo_LogHandler* user_handler);
static void Loggo_DestroyLogHandler(Loggo_LogHandler* handler);
static void Loggo_DestroyLogFormat(Loggo_LogFormat* format);
static void Loggo_CleanUpLogger(Loggo_Logger* logger);
static void Loggo_HandleLogMessage(Loggo_LogMessage* message, Loggo_LogFormat* format, Loggo_LogHandler* handler);

// Hash Table
static Loggo_Logger* Loggo_HTFindItem(const char* name);
static int32_t Loggo_HTInsertItem(const char* name, Loggo_Logger* logger);
static void Loggo_HTDeleteItem(const char* name);
static void Loggo_HTResizeTable();
static void Loggo_HTInitTable();
static void Loggo_HTDeleteTable();
static int32_t Loggo_StringHash(const char* name, const int32_t prime, const int32_t buckets);
static int32_t Loggo_LoggerStringHash(const char* name, const int32_t buckets, const int32_t attempt);



////////////////////////////////////
// Api
////////////////////////////////////


// Create logger components, start thread with the created queue
// Return -1 on error dont allocate anything, clean slate
LOGGODEF int32_t Loggo_CreateLogger(const char* name, Loggo_LogFormat* user_format, Loggo_LogHandler* user_handler) {
    #ifdef MINT__DEBUG
        assert(name);
    #endif

    if (!name) {
        return -1;
    }

    Loggo_HTInitTable();

    Loggo_Logger* logger = LOGGO_MALLOC(sizeof(Loggo_Logger));
    memset(logger, 0U, sizeof(*logger));

    // Fill up with info
    logger->handler = Loggo_CreateLogHandler(user_handler);

    // Clean up and return -1
    if (!logger->handler) {
        Loggo_DestroyLogHandler(logger->handler);
        memset(logger, 0U, sizeof(*logger));
        LOGGO_FREE(logger);
        logger = NULL;
        return  -1;
    }

    logger->format = Loggo_CreateLogFormat(user_format);
    logger->name = name;
    logger->queue = Loggo_CreateQueue(logger->format->queue_capacity);

    // Handle the string allocation to a logger id
    int32_t id = Loggo_HTInsertItem(name, logger);

    // We failed
    if (id == -1) {
        Loggo_DestroyLogFormat(logger->format);
        Loggo_DestroyLogHandler(logger->handler);
        Loggo_DestroyQueue(logger->queue);
        memset(logger, 0U, sizeof(*logger));
        LOGGO_FREE(logger);
        logger = NULL;
        return  id;
    }

    // Handle new stuff
    logger->id = id;

    // Spin up a thread for the loggers
    LOGGO_THREAD_CREATE(&logger->thread_id, Loggo_RunLogger, ((void*)logger));

    // Return Id to user
    return logger->id;
}


// Shutdown the loggers by iterating and setting values
LOGGODEF void Loggo_DeleteLoggers() {
    for (int32_t idx = 0; idx < LOGGO_LOGGER_HASH_TABLE.capacity; idx++) {
        // Delete non null items by grabbing there names from the table
        if (LOGGO_LOGGER_HASH_TABLE.loggers[idx] != NULL && LOGGO_LOGGER_HASH_TABLE.loggers[idx] != &LOGGO_LOGGER_DELETED) {
            Loggo_HTDeleteItem(LOGGO_LOGGER_HASH_TABLE.loggers[idx]->name);
        }
    }

    Loggo_HTDeleteTable();
    memset(&LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(Loggo_HashTable));
}

// Shutdown the loggers by iterating and setting values
LOGGODEF void Loggo_DeleteLogger(const char* name) {
    Loggo_HTDeleteItem(name);

    // Just delete the table and clear it so its inited next time
    if (LOGGO_LOGGER_HASH_TABLE.size == 0) {
        Loggo_HTDeleteTable();
        memset(&LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(Loggo_HashTable));
    }
}



// Log message with Enqueue
LOGGODEF void Loggo_Log(const char* name, Loggo_LogLevel level, const char* msg) {
    #ifdef MINT__DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    Loggo_Logger* logger = Loggo_HTFindItem(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        Loggo_DeleteLoggers();
        exit(EXIT_FAILURE);
    }

    Loggo_LogMessage* message = Loggo_CreateLogMessage(logger, level, msg);


    Loggo_Enqueue(logger->queue, message);
}


// Log message with Enqueue, optionally free msg if free_string is true. In case the user wants to pass
// a malloced string in
LOGGODEF void Loggo_Log2(const char* name, Loggo_LogLevel level, char* msg, bool free_string) {
    #ifdef MINT__DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    Loggo_Logger* logger = Loggo_HTFindItem(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        Loggo_DeleteLoggers();
        exit(EXIT_FAILURE);
    }

    Loggo_LogMessage* message = Loggo_CreateLogMessage(logger, level, msg);

    if (free_string) {
        free(msg);
    }
    
    Loggo_Enqueue(logger->queue, message);
}


// Queue

// Create the queue with sane defaults
static Loggo_LogQueue* Loggo_CreateQueue(uint32_t capacity) {
    #ifdef MINT__DEBUG
        assert(capacity > 0U);
    #endif

    Loggo_LogQueue* queue = LOGGO_MALLOC(sizeof(Loggo_LogQueue));

    // Clear out values and set actual ones
    memset(queue, 0U, sizeof(*queue));
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->size = 0U;

    // Init locks/cond
    LOGGO_MUTEX_INIT(queue->queue_lock);
    LOGGO_COND_INIT(queue->queue_not_full);
    LOGGO_COND_INIT(queue->queue_not_empty);

    // Init messsages circular buffer
    queue->messages = LOGGO_MALLOC(sizeof(Loggo_LogMessage*) * queue->capacity);
    memset(queue->messages, 0U, sizeof(Loggo_LogMessage*) * queue->capacity);
    return queue;
}


// Free messages, queue and zero out mem
static void Loggo_DestroyQueue(Loggo_LogQueue* queue) {
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
                LOGGO_FREE(queue->messages[start]->msg);
                queue->messages[start]->msg = NULL;
            }
            LOGGO_FREE(queue->messages[start]);
            queue->messages[start] = NULL;
        }
        
        // Wraparound
        start = (start + 1) % queue->capacity;
    }

    // Clean up threading stuff
    LOGGO_MUTEX_DESTROY((queue->queue_lock));
    LOGGO_COND_DESTROY((queue->queue_not_empty));
    LOGGO_COND_DESTROY((queue->queue_not_full));

    // Free messages that the queue owns
    if (queue->messages) {
        LOGGO_FREE(queue->messages);
        queue->messages = NULL;
    }

    // Clean up last bits of memory
    memset(queue, 0U, sizeof(*queue));
    LOGGO_FREE(queue);
}


// If head + 1 == tail
static bool Loggo_IsQueueFull(Loggo_LogQueue* queue) {
    return queue->head + 1 == queue->tail;
}

 
// If head == tail
static bool Loggo_IsQueueEmpty(Loggo_LogQueue* queue) {
    return queue->head == queue->tail;
}


// Wait for the queue to not be full
// Add message
// Signal that its not empty anymore
static void Loggo_Enqueue(Loggo_LogQueue* queue, Loggo_LogMessage* message) {
    LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef MINT__DEBUG
        assert(queue);
        assert(message);
    #endif
    
    // Just dont queue if full
    while (Loggo_IsQueueFull(queue)) {
        LOGGO_COND_WAIT(queue->queue_not_full, queue->queue_lock);
    }
    
    // Add message and advance queue
    queue->messages[queue->head] = message;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size++;

    // Let the thread know it has a message
    LOGGO_COND_SIGNAL(queue->queue_not_empty);
    LOGGO_MUTEX_UNLOCK(queue->queue_lock);
}


// If the queue is empty just wait until we get the okay from Enqueue
// Also let enqueue know we are not full because we took a message
static Loggo_LogMessage* Loggo_Dequeue(Loggo_LogQueue* queue) {
    LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef MINT__DEBUG
        assert(queue);
    #endif

    while (Loggo_IsQueueEmpty(queue)) {
        LOGGO_COND_WAIT(queue->queue_not_empty, queue->queue_lock);
    }

    Loggo_LogMessage* message = queue->messages[queue->tail];
    queue->messages[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size--;

    LOGGO_COND_SIGNAL(queue->queue_not_full);
    LOGGO_MUTEX_UNLOCK(queue->queue_lock);
    return message; 
}


// Logging


// Stringify Level
static char* Loggo_StringFromLevel(Loggo_LogLevel level) {
    char* result = NULL;
    switch(level) {
        case LOGGO_LEVEL_DEBUG:
            result = "DEBUG";
            break;
        case LOGGO_LEVEL_INFO:
            result = "INFO";
            break;
        case LOGGO_LEVEL_WARN:
            result = "WARN";
            break;
        case LOGGO_LEVEL_ERROR:
            result = "ERROR";
            break;
        case LOGGO_LEVEL_FATAL:
            result = "FATAL";
            break;
        default:
            result = "UNKNOWN";
            break;
    }

    return result;
}


// Colorify Level
static char* Loggo_ColorFromLevel(Loggo_LogLevel level) {
    char* result = NULL;
    switch(level) {
        case LOGGO_LEVEL_DEBUG:
            result = LOGGO_GREEN;
            break;
        case LOGGO_LEVEL_INFO:
            result = LOGGO_CYAN;
            break;
        case LOGGO_LEVEL_WARN:
            result = LOGGO_YELLOW;
            break;
        case LOGGO_LEVEL_ERROR:
            result = LOGGO_RED;
            break;
        case LOGGO_LEVEL_FATAL:
            result = LOGGO_MAGENTA;
            break;
        default:
            result = LOGGO_WHITE;
            break;
    }

    return result;
}


static Loggo_LogFormat* Loggo_CreateLogFormat(Loggo_LogFormat* user_format) {
    #ifdef MINT__DEBUG
        assert(user_format);
    #endif

    // Create User handler
    Loggo_LogFormat* log_format = LOGGO_MALLOC(sizeof(Loggo_LogFormat));
    memset(log_format, 0U, sizeof(*log_format));

    // Copy user format if it exists
    if (user_format) { 
        memcpy(log_format, user_format, sizeof(*user_format));
    }

    // Set defaults
    // Level defaults to LOG_DEBUG
    // colors defaults to 0 (false)
    // flush defaults to 0 (false)

    if (!log_format->linesep) log_format->linesep = LOGGO_DEFAULT_LINE_SEP;
    if (log_format->queue_capacity == 0) log_format->queue_capacity = LOGGO_DEFAULT_QUEUE_SIZE;
    if (!log_format->time_format) log_format->time_format = LOGGO_DEFAULT_TIME_FORMAT;
    if (!log_format->linebeg) log_format->linebeg = LOGGO_DEFAULT_LINE_BEG;
    return log_format;
}


static Loggo_LogHandler* Loggo_CreateLogHandler(Loggo_LogHandler* user_handler) {

    // Create User handler
    Loggo_LogHandler* log_handler = LOGGO_MALLOC(sizeof(Loggo_LogHandler));
    memset(log_handler, 0U, sizeof(*log_handler));

    // Copy user format if it exists
    if (user_handler) { 
        memcpy(log_handler, user_handler, sizeof(*user_handler));

        // You need the handle
        if (!log_handler->handle) {
            Loggo_DestroyLogHandler(log_handler);
            return NULL;
        }

        // Set NULL handlers if they are not there
        if(!log_handler->write_handler) log_handler->write_handler = Loggo_NullWrite;
        if(!log_handler->flush_handler) log_handler->flush_handler = Loggo_NullFlush;
        if(!log_handler->close_handler) log_handler->close_handler = Loggo_NullClose;

    } else {
        // Defaults
        log_handler->handle = stdout;
        log_handler->write_handler = Loggo_StreamWrite;
        log_handler->close_handler = Loggo_StreamClose;
        log_handler->flush_handler = Loggo_StreamFlush;
    }

    return log_handler;  
}


static void Loggo_DestroyLogHandler(Loggo_LogHandler* handler) {
    #ifdef MINT__DEBUG
        assert(handler);
    #endif

    memset(handler, 0U, sizeof(*handler));
    LOGGO_FREE(handler);  
}


static void Loggo_DestroyLogFormat(Loggo_LogFormat* format) {
    #ifdef MINT__DEBUG
        assert(format);
    #endif

    memset(format, 0U, sizeof(*format));
    LOGGO_FREE(format);
}


// Actual ouptut of message and cleanup
static void Loggo_HandleLogMessage(Loggo_LogMessage* message, Loggo_LogFormat* format, Loggo_LogHandler* handler) {
    #ifdef MINT__DEBUG
        assert(message);
        assert(format);
        assert(handler);
    #endif

    if (message->level >= format->level) {
        if (format->colors) {
            handler->write_handler(Loggo_ColorFromLevel(message->level), handler->handle);
        }

        // Write actual output
        handler->write_handler(format->linebeg, handler->handle);
        handler->write_handler(" ", handler->handle);
        handler->write_handler(message->msg, handler->handle);
        handler->write_handler(format->linesep, handler->handle);

        // Reset colors
        if (format->colors) {
            handler->write_handler(LOGGO_RESET, handler->handle);
        }

        // Flush if needed
        if (format->flush) {
            handler->flush_handler(handler->handle);
        }
    }

    // Clean up message
    LOGGO_FREE(message->msg);
    message->msg = NULL;

    // Clean up message
    LOGGO_FREE(message);
}


// Thread spawned handler of messages
static void* Loggo_RunLogger(void* arg) {
    #ifdef MINT__DEBUG
        assert(arg);
    #endif
    Loggo_Logger* logger = arg;
    #ifdef MINT__DEBUG
        assert(logger->queue);
    #endif

    // For some reason you have to grab the read lock and read all that you can in a loop
    // Or else the condition is never signaled and you wait
    while (!logger->done) {
        Loggo_LogMessage* message = Loggo_Dequeue(logger->queue);

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
            Loggo_HandleLogMessage(message, logger->format, logger->handler);
        }
    }

    return EXIT_SUCCESS;
}


// Create a nice formatted log message
static Loggo_LogMessage* Loggo_CreateLogMessage(Loggo_Logger* logger, Loggo_LogLevel level, const char* msg) {
    // Misc
    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;

    // Level String
    const char* level_string = Loggo_StringFromLevel(level);
    uint32_t level_size = strlen(level_string);

    // Get time
    char time_buffer[128U];
    time_t current_time = time(NULL);
    time_buffer[strftime(time_buffer, sizeof(time_buffer), logger->format->time_format, localtime(&current_time))] = '\0';

    // Create LogMessage
    Loggo_LogMessage* message = LOGGO_MALLOC(sizeof(Loggo_LogMessage));
    memset(message, 0U, sizeof(*message));
    
    // Insert formatted message inside of LogMessage
    formatted_msg = LOGGO_MALLOC(strlen(msg) + strlen(time_buffer) + level_size + padding + 1U);
    sprintf(formatted_msg, "[%s] %s %s", time_buffer, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    return message;
}


// Free all the handles
static void Loggo_CleanUpLogger(Loggo_Logger* logger) {

    // Queue up final message and wait for logger to close
    Loggo_Enqueue(logger->queue, &LOGGO_LOGGER_TERMINATE);
    LOGGO_THREAD_JOIN(logger->thread_id);

    // Free handles
    Loggo_DestroyLogHandler(logger->handler);
    logger->handler = NULL;

    Loggo_DestroyLogFormat(logger->format);
    logger->format = NULL;

    Loggo_DestroyQueue(logger->queue);
    logger->queue = NULL;

    LOGGO_FREE(logger);
}


// Logger hash table


// Try to find an item returning NULL if not found
static Loggo_Logger* Loggo_HTFindItem(const char* name) {
    int32_t hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    Loggo_Logger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL) {
        if (current_logger != &LOGGO_LOGGER_DELETED) {
            if (strcmp(name, current_logger->name) == 0) {
                return current_logger;
            }
        }
        hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }

    return NULL;
}


// Try to insert an item, resize if needed
static int32_t Loggo_HTInsertItem(const char* name, Loggo_Logger* logger) {
    // Try a resize
    Loggo_HTResizeTable();
    int32_t hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    Loggo_Logger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL && current_logger != &LOGGO_LOGGER_DELETED) {
        hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        // If the item exists update it
        if (current_logger) {
            if (strcmp(name, current_logger->name) == 0) {
                Loggo_CleanUpLogger(current_logger);
                LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
                return logger->id;
            }
        }

        attempt++;

    }

    LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
    LOGGO_LOGGER_HASH_TABLE.size++;
    logger->id = hash;
    return logger->id;
}


static void Loggo_HTDeleteItem( const char* name) {
    int32_t hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    Loggo_Logger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    int32_t attempt = 1;
    while(current_logger != NULL) {
        if (current_logger != &LOGGO_LOGGER_DELETED && current_logger != NULL) {
            if (strcmp(current_logger->name, name) == 0) {
                Loggo_CleanUpLogger(current_logger);
                LOGGO_LOGGER_HASH_TABLE.loggers[hash] = &LOGGO_LOGGER_DELETED;
            }
        }
        hash = Loggo_LoggerStringHash(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger =  LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }
    LOGGO_LOGGER_HASH_TABLE.size--; 
}


// Just realloc space up
static void Loggo_HTResizeTable() {
    int32_t load = (LOGGO_LOGGER_HASH_TABLE.size * 100) / LOGGO_LOGGER_HASH_TABLE.capacity;
    if (load > (LOGGO_LOGGER_HASH_TABLE.load_factor * 100)) {
        LOGGO_LOGGER_HASH_TABLE.capacity *= 2;

        // No need to realloc a new table
        // Just realloc memory chunk
        LOGGO_LOGGER_HASH_TABLE.loggers = LOGGO_REALLOC(LOGGO_LOGGER_HASH_TABLE.loggers, sizeof(Loggo_Logger*) * LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Delete table and set defaults
static void Loggo_HTDeleteTable() {
    if (LOGGO_LOGGER_HASH_TABLE.loggers) {
        LOGGO_FREE(LOGGO_LOGGER_HASH_TABLE.loggers);
        LOGGO_LOGGER_HASH_TABLE.loggers = NULL;
    }

    LOGGO_LOGGER_HASH_TABLE.capacity = LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
    LOGGO_LOGGER_HASH_TABLE.size = 0;
    LOGGO_LOGGER_HASH_TABLE.load_factor = LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
}


// Init whats needed
static void Loggo_HTInitTable() {
    if (!LOGGO_LOGGER_HASH_TABLE.loggers) {
        LOGGO_LOGGER_HASH_TABLE.capacity = LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
        LOGGO_LOGGER_HASH_TABLE.size = 0;
        LOGGO_LOGGER_HASH_TABLE.load_factor = LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
        LOGGO_LOGGER_HASH_TABLE.loggers = LOGGO_MALLOC(sizeof(Loggo_Logger*) * LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Simple hash
static int32_t Loggo_StringHash(const char* name, const int32_t prime, const int32_t buckets) {
    int32_t hash = 0;
    const int len = strlen(name);
    for (int32_t i = 0; i < len; i++) {
        hash += (int32_t)pow(prime, len - (i+1)) * name[i];
        hash = hash % buckets;
    }
    return hash; 
}


// Double hash
static int32_t Loggo_LoggerStringHash(const char* name, const int32_t buckets, const int32_t attempt) {
    const int32_t hasha = Loggo_StringHash(name, LOGGO_PRIME_1, buckets);
    const int32_t hashb = Loggo_StringHash(name, LOGGO_PRIME_2, buckets);
    return (hasha + (attempt * (hashb + 1))) % buckets;
}


#endif // LOGGO_IMPLEMENTATION

#endif // LOGGO_H
