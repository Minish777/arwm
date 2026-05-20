#ifndef KEYBINDS_H
#define KEYBINDS_H

#include <xcb/xcb.h>

void keybinds_grab_all(void);
void keybinds_handle(xcb_key_press_event_t *ev);

#endif
