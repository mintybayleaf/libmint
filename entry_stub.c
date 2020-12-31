/* Uncomment defines with components you actually want  */

#define USE_LOGGO
#define USE_EZEMU


// Loggo Lib
#ifdef USE_LOGGO
    #define LOGGO_IMPLEMENTATION
    #include "loggo.h"
#endif

// Ezemu Lib
#ifdef USE_EZEMU
    #define EZEMU_IMPLEMENTATION
    #include "ezemu.h"
#endif
