#ifndef LOGGING_H
#define LOGGING_H

#include <syslog.h>

extern int log_fd;
extern int log_level;

extern void logit(int , char* , ... );

#endif
