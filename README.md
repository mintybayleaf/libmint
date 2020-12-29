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
    `FILENAME_DEF_STATIC`.


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


2. CoolDisplay (cooldisplay.h)
    - SDL2 wrapper for creating useable windows
    - Better drawing methods
    - Helpful wrappers
    - Predefined window types
    - Vulkan wrappers


3. CrossAudio (crossaudio.h)
    - Simple audio hardware device wrapper
    - Query audio devices and get handles to them
    - Send WAV audio to audio devices
    - Wave generation functions/types
    - Good for generating sounds


