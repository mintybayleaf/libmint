# libmint

## Libraries for making C development a little easier


I was inspired by [stb](https://github.com/nothings/stb) to create single header libraries.


Basically just copy the header into your project and use a define statement for the one time definitions.

    The `libs` folder contains a single header only library. In order to use the library you naturally just have to include
    the .h file wherever you use it in your own project. Yep just copy the file over! 
    You must only define `MINT_FILENAME_IMPLEMENTATION` in the translation unit you want the functions to be defined,
    not just decalared. If you dont do this you will encounter linker errors for unresolved defintions.
    You can change whether the api is declared static or extern which would scope the library to one translation unit by defining
    `MINT_FILENAME_DEF_STATIC`.

You will notice a lot of copied cross/platform code in this library. I did this so users could use a single header file without having to worry about other things.
It was a concious choice I made. Ease of use in C is important to me and anyone can get started just by copying a file, defining values and compiling their code.

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


## Header Installation / Usage

Just copy the header files of the libs you want.
Add the appropiate `MINT_FILENAME_IMPLEMENTATION` and maybe `MINT_FILENAME_DEF_STATIC`.
If the library has extra dependencies just compile those in as well.

##  Run the logging example

```console
cmake -H. -Bbuild
cmake --build build --target mint_loggo_example
./build/bin/mint_loggo_examples
```

### Screenshot

![Mint Loggo](images/mint_loggo_example.png)
