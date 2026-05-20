#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

#define MAX_WINDOWS 128

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void spawn(const char *cmd);

#endif
