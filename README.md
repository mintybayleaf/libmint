# libmint

## Libraries for making C development a little easier


I was inspired by [stb](https://github.com/nothings/stb) to create single header libraries.


You have two options with this library.


1.
    The `libs` folder contains a single header only library. In order to use the library you naturally just have to include
    the .h file wherever you use it in your own project. Yep just copy the file over! 
    You must only define `FILENAME_IMPLEMENTATION` in the translation unit you want the functions to be defined,
    not just decalared. If you dont do this you will encounter linker errors for unresolved defintions.
    You can change whether the api is declared static or extern which would scope the library to one translation unit by defining
    `MINT_DEF_STATIC`.


2.
    The other alternative is to use the cmake to generate libmint which stub all of this out and combine the object files.
    You can use cmake to install the library and then link to it statically or dynamically. The header files will be installed under
    `<mint/filename.h>` where `filename` is the headerfile name. 
    This way is nice because everything is bundled up nicely.


You will notice a lot of copied cross/platform code in this library. I did this so users could use a single header file without having to worry about other things.
It was a concious choice I made. Ease of use in C is important to me and anyone can get started just by copying a file, defining values and compiling there code.


Some libraries have dependencies. `CoolDisplay` for example has SDL2. Dependencies are listed inside the library folder. I tried to minimize dependencies for example with `Loggo` there are dependencies on only the stdc library and posix/winthreads which are installed on systems by default.


NOTE: These are tested on Windows 10 with MSVC 2019, OSX Big Sur and Debian Linux


## Current Libraries


1. Loggo (loggo.h)
    - Logging library
    - Uses threads with a blocking queue (conditions/mutex) to gaurantee all messages are processed
    - Uses a hashtable for quick logger lookup
    - Cleanup code flushes messages in queue and waits until all the logs are emitted
    - Configurable log format with colors, flushing, time strings and more
    - Configurable output handler
    - Convenience logging macros


## In Progress


2. EzEmu (ezemu.h)
    - SDL2 code for creating emulator displays
    - Texture rendering for pixel buffers
    - Simple 2D emulators
    - Simple useful components/prebuilt windows
    - Audio hardware support


## Installation


Using CMake 3.13.4 or later you can do the following


```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/my-install-prefix
cmake --build build --target mint
```


NOTE: CMAKE_INSTALL_PREFIX must reflect how you want your compiler to search for the files.


For instance


If I install the files in `$HOME/opt` I would do this...


```bash
export LD_LIBRARY_PATH="$HOME/opt/lib:$LD_LIBRARY_PATH" # for libs
export CPATH="$HOME/opt/include:$CPATH" # for includes
```


Or perhaps set them in your shell

## Usage

You can use the library by now using the libmint and the header path `<mint/filename.h>`
where `filename` is the same as the specific header file.


```bash
gcc -c myproject.c -o myproject -lmint
```


Example code

```c
#define LOGGO_IMPLEMENTATION
#define LOGGO_USE_HELPERS
#include <mint/loggo.h>

// FILE*
#include <stdio.h>

// int32_t
#include <stdint.h>

// Loggers accessed by name
const char *const stdout_logger = "stdout";
const char *const file_logger = "file_logger";

int main() {
    // Custom Format
    // NULL Handler defaults to stdout
    int32_t stdout_id = Loggo_CreateLogger(stdout_logger, 
                            &(Loggo_LogFormat){.colors=true, .level=LOGGO_LEVEL_DEBUG, .flush=true, .time_format="%Y-%M-%D", .linebeg="[LOG STDOUT]", .linesep="\n"},
                            NULL);

    // WriteStream uses fputs
    // CloseStream uses fclose
    // FlushStream uses fflush
    // Obviously you can customize these by matching the expected function typedefs
    FILE* file = fopen("mylog.txt", "w");
    int32_t file_id = Loggo_CreateLogger(file_logger, 
                            &(Loggo_LogFormat){.colors=false, .level=LOGGO_LEVEL_DEBUG, .flush=true, .time_format="%Y-%M-%D", .linebeg="[LOG FILE]", .linesep="\n"},
                            &(Loggo_LogHandler){.handle=file, .write_handler=Loggo_StreamWrite, .close_handler=Loggo_StreamClose, .flush_handler=Loggo_StreamFlush});

    // This would happen if you failed to supply a handle to a handler that you specified for instance
    if (stdout_id == -1 || file_id == -1) {
        Loggo_DeleteLoggers();
        fprintf(stderr, "Could not init logger..... Exiting");
        exit(EXIT_FAILURE);
    }

    LOG_DEBUG(stdout_logger, "Hello Debug");
    LOG_INFO(stdout_logger, "Hello Info");
    LOG_WARN(stdout_logger, "Hello Warn");
    LOG_ERROR(stdout_logger, "Hello Error");
    LOG_FATAL(stdout_logger, "Hello Fatal");

    LOG_DEBUG(file_logger, "Hello Debug");
    LOG_INFO(file_logger, "Hello Info");
    LOG_WARN(file_logger, "Hello Warn");
    LOG_ERROR(file_logger, "Hello Error");
    LOG_FATAL(file_logger, "Hello Fatal");

    // Or
    Loggo_Log(file_logger, LOGGO_LEVEL_ERROR, "AHHHHH HELP");

    // Or
    char* msg = malloc(sizeof(char) * 128U);
    snprintf(msg, (sizeof(char) * 128U), "Custom Message 0x%8X", 0xDEADBEEF);
    // Passing true in Log2 frees the msg object
    Loggo_Log2(file_logger, LOGGO_LEVEL_FATAL, msg, true);
    // LOG2_LEVEL also works

    // Delete one logger
    Loggo_DeleteLogger(file_logger); 

    // Call at end of program to delete all loggers and clean up
    Loggo_DeleteLoggers();
    return 0;
}
```


## Usage in another cmake project