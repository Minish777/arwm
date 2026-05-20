#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb.h>

void events_loop(void);
void events_handle(xcb_generic_event_t *ev);

#endif
