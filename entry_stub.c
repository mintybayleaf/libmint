/* Define/Undefine components you want to use */

#define USE_MINT
#define USE_MINT_LOGGO
/* #define USE_MINT_EZEMU */


// Mint utils Lib
#ifdef USE_MINT
    #define MINT_IMPLEMENTATION
    #include "mint.h"
#endif

// Loggo Lib
#ifdef USE_MINT_LOGGO
    #define MINT_LOGGO_IMPLEMENTATION
    #include "mint_loggo.h"
#endif

// Ezemu Lib
#ifdef USE_MINT_EZEMU
    #define MINT_EZEMU_IMPLEMENTATION
    #include "mint_ezemu.h"
#endif
