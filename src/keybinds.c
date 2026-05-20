#include <stdlib.h>
#include <X11/keysym.h>
#include "arwm.h"

void keybinds_grab_all(void) {
    xcb_key_symbols_t *syms = xcb_key_symbols_alloc(conn);
    struct { xcb_keysym_t sym; uint16_t mod; } binds[] = {
        { XK_Return, XCB_MOD_MASK_4 },
        { XK_d, XCB_MOD_MASK_4 },
        { XK_q, XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT },
        { XK_c, XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT }
    };

    for (size_t i = 0; i < sizeof(binds)/sizeof(binds[0]); i++) {
        xcb_keycode_t *keycode = xcb_key_symbols_get_keycode(syms, binds[i].sym);
        if (keycode) {
            for (int j = 0; keycode[j] != XCB_NO_SYMBOL; j++) {
                xcb_grab_key(conn, 1, screen->root, binds[i].mod, keycode[j], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
            }
            free(keycode);
        }
    }
    xcb_key_symbols_free(syms);
}

void keybinds_handle(xcb_key_press_event_t *ev) {
    xcb_key_symbols_t *syms = xcb_key_symbols_alloc(conn);
    xcb_keysym_t keysym = xcb_key_symbols_get_keysym(syms, ev->detail, 0);
    if (keysym == XK_Return && (ev->state & XCB_MOD_MASK_4)) spawn(cfg.terminal);
    else if (keysym == XK_d && (ev->state & XCB_MOD_MASK_4)) spawn(cfg.launcher);
    else if (keysym == XK_q && (ev->state & (XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT))) exit(0);
    else if (keysym == XK_c && (ev->state & (XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT))) {
        if (clients) xcb_kill_window(conn, clients->win);
    }
    xcb_key_symbols_free(syms);
}
