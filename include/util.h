#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void spawn(const char *cmd);

#endif
