
// This creates the macros for you, or doesnt if not defined
#define MINT_LOGGO_USE_HELPERS
#define MINT_LOGGO_IMPLEMENTATION
#include "mint_loggo.h"

// This shouldnt remimplement it which would cause mutiple defintion linker errors
// Just a test
#include "mint_loggo.h"

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
    int32_t stdout_id = Mint_Loggo_CreateLogger(stdout_logger, 
                            &(Mint_Loggo_LogFormat){.colors=true, .level=MINT_LOGGO_LEVEL_DEBUG, .flush=true, .linebeg="[LOG STDOUT]", .linesep="\n"},
                            NULL);

    // WriteStream uses fputs
    // CloseStream uses fclose
    // FlushStream uses fflush
    // Obviously you can customize these by matching the expected function typedefs
    FILE* file = fopen("mylog.txt", "w");
    int32_t file_id = Mint_Loggo_CreateLogger(file_logger, 
                            &(Mint_Loggo_LogFormat){.colors=false, .level=MINT_LOGGO_LEVEL_DEBUG, .flush=true, .time_format="%Y-%M-%D", .linebeg="[LOG FILE]", .linesep="\n"},
                            &(Mint_Loggo_LogHandler){.handle=file, .write_handler=Mint_Loggo_StreamWrite, .close_handler=Mint_Loggo_StreamClose, .flush_handler=Mint_Loggo_StreamFlush});

    // This would happen if you failed to supply a handle to a handler that you specified for instance
    if (stdout_id == -1 || file_id == -1) {
        Mint_Loggo_DeleteLoggers();
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
    Mint_Loggo_Log(file_logger, MINT_LOGGO_LEVEL_ERROR, "AHHHHH HELP");

    // Or
    char* msg = malloc(sizeof(char) * 128U);
    snprintf(msg, (sizeof(char) * 128U), "Custom Message 0x%8X", 0xDEADBEEF);
    // Passing true in Log2 frees the msg object
    Mint_Loggo_Log2(file_logger, MINT_LOGGO_LEVEL_FATAL, msg, true);
    // LOG2_LEVEL also works

    // Delete one logger
    Mint_Loggo_DeleteLogger(file_logger); 

    // Call at end of program to delete all loggers and clean up
    Mint_Loggo_DeleteLoggers();
    return 0;
}