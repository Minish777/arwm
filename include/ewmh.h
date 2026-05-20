#ifndef EWMH_H
#define EWMH_H

#include <xcb/xcb_ewmh.h>

void ewmh_init(void);
void ewmh_update_client_list(void);
void ewmh_update_active_window(xcb_window_t win);

#endif
