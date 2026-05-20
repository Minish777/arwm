#include <stdlib.h>
#include <stdio.h>
#include "arwm.h"

void arwm_init(void) {
    int scr_num;
    conn = xcb_connect(NULL, &scr_num);
    if (xcb_connection_has_error(conn)) {
        log_error("Could not connect to X server.");
        exit(1);
    }

    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for (int i = 0; i < scr_num; ++i) xcb_screen_next(&iter);
    screen = iter.data;

    uint32_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                    XCB_EVENT_MASK_PROPERTY_CHANGE;

    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, &mask);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
        log_error("Another window manager is already running.");
        free(err);
        exit(1);
    }

    config_load_defaults();
    config_load();
    ewmh_init();
    monitor_init();
    keybinds_grab_all();
    ipc_init();

    xcb_flush(conn);
}

void arwm_cleanup(void) {
    ipc_cleanup();
    monitor_cleanup();
    if (conn) {
        xcb_disconnect(conn);
    }
}
