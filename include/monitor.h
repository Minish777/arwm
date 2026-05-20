#ifndef MONITOR_H
#define MONITOR_H

#include <xcb/xcb.h>

typedef struct Monitor {
    int x, y, w, h;
    struct Monitor *next;
} Monitor;

extern Monitor *monitors;

void monitor_init(void);
void monitor_cleanup(void);

#endif
