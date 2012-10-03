// Logging
// a simple way to print information to the screen or a stream depending on which;



#include "jit_logging.h"
#include <string>
#include <stdio.h>
#include <stdarg.h>

#define GLOBAL_LOG_LEVEL 11 // set < DEBUG for no logging!


const char* log_level_text_full(LOG_LEVEL level) {
    const char* buff[8];
    
    switch(level) {
        case LOG_ERROR: *buff = "ERROR"; break;
        case LOG_WARNING: *buff = "WARNING"; break;
        case LOG_INFO: *buff = "INFO"; break;
        case LOG_DEBUG: *buff = "DEBUG"; break;
        default: *buff = "none" ; break;
    }
    return *buff;
}

const char* log_level_text(LOG_LEVEL level) {
    const char* buff[2];
    
    switch(level) {
        case LOG_ERROR: *buff = "E"; break;
        case LOG_WARNING: *buff = "W"; break;
        case LOG_INFO: *buff = "I"; break;
        case LOG_DEBUG: *buff = "D"; break;
        default: *buff = "-" ; break;
    }
    return *buff;
}

void _log_list(LOG_LEVEL level, const char * format, va_list args) {    
    
    if (GLOBAL_LOG_LEVEL >= level ) {
        fprintf(stdout, "%s: ",log_level_text(level));
        vfprintf(stdout,format,args);    
    }   
    
    
}

void log(LOG_LEVEL level, const char * format, ...) {        
    va_list args;        
    va_start(args, format);
    _log_list(level,format,args);    
    va_end (args);
}


void logError(const char * format, ...) {        
    va_list args;        
    va_start(args, format);
    _log_list(LOG_ERROR,format,args);    
    va_end (args);
}
void logWarning(const char * format, ...) {        
    va_list args;        
    va_start(args, format);
    _log_list(LOG_WARNING,format,args);    
    va_end (args);
}
void logInfo(const char * format, ...) {        
    va_list args;        
    va_start(args, format);
    _log_list(LOG_INFO,format,args);    
    va_end (args);
}
void logDebug(const char * format, ...) {        
    va_list args;        
    va_start(args, format);
    _log_list(LOG_DEBUG,format,args);    
    va_end (args);
}
