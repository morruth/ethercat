#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "logging.h"

/* now just log to stderr */
int log_fd=STDERR_FILENO;
int log_level=LOG_ERR;

void logit(int level, char* format, ... ){
    va_list argptr;
    va_start(argptr,format);

    if(level<=log_level){
        vdprintf(log_fd,format, argptr);
    }
}
