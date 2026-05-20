#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "arwm.h"

void log_info(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "ARWM [INFO]: "); vfprintf(stderr, fmt, ap); fprintf(stderr, "\n");
    va_end(ap);
}

void log_error(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "ARWM [ERROR]: "); vfprintf(stderr, fmt, ap); fprintf(stderr, "\n");
    va_end(ap);
}

void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        exit(0);
    }
}
