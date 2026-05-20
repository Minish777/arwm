#ifndef ARWM_H
#define ARWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_keysyms.h>

#include "config.h"
#include "monitor.h"
#include "client.h"
#include "layout.h"
#include "events.h"
#include "ipc.h"
#include "ewmh.h"
#include "keybinds.h"
#include "util.h"

extern xcb_connection_t *conn;
extern xcb_screen_t *screen;
extern xcb_ewmh_connection_t ewmh;

void arwm_init(void);
void arwm_cleanup(void);

#endif
