// jit_logging.h


#ifndef _JIT_LOGGING_H
#define _JIT_LOGGING_H


#include <stdio.h>
#include <iostream>
#include <string.h>

enum LOG_LEVEL {
    LOG_ERROR = 10,
    LOG_WARNING = 20,
    LOG_INFO = 30,
    LOG_DEBUG = 40    
} ;

void log(LOG_LEVEL level, const char * format, ...);
void logDebug(const char * format, ...);
void logInfo(const char * format, ...);
void logWarning(const char * format, ...);
void logError(const char * format, ...);


void logcustom(bool* level, int index, const char * format, ...);
#endif
