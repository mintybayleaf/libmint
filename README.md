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
    `MINT_FILENAME_DEF_STATIC`.


2.
    The other alternative is to use the cmake to generate libmint which stub all of this out and combine the object files.
    You can use cmake to install the library and then link to it statically or dynamically. The header files will be installed under
    `<mint/filename.h>` where `filename` is the headerfile name. 
    This way is nice because everything is bundled up nicely.


You will notice a lot of copied cross/platform code in this library. I did this so users could use a single header file without having to worry about other things.
It was a concious choice I made. Ease of use in C is important to me and anyone can get started just by copying a file, defining values and compiling there code.


Some libraries have dependencies. `EzEmu` for example has SDL2. Dependencies are listed inside the library folder. I tried to minimize dependencies for example with `Loggo` there are dependencies on only the stdc library and posix/winthreads which are installed on systems by default.


NOTE: These were tested on MSVC 2019, GCC and Clang


## Current Libraries


1. Loggo (mint_loggo.h)
    - Logging library
    - Uses threads with a blocking queue (conditions/mutex) to gaurantee all messages are processed
    - Uses a hashtable for quick logger lookup
    - Cleanup code flushes messages in queue and waits until all the logs are emitted
    - Configurable log format with colors, flushing, time strings and more
    - Configurable output handler
    - Convenience logging macros


## Installation

Just copy the header files of the libs you want or.....

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

## Usage in another cmake project

Cloning the repo and using add_subdirectory is pretty swell.