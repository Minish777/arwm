#include <stdlib.h>
#include "arwm.h"

void ewmh_init(void) {
    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(conn, &ewmh);
    if (!xcb_ewmh_init_atoms_replies(&ewmh, cookie, NULL)) {
        log_error("Failed to initialize EWMH atoms.");
    }

    xcb_window_t win = xcb_generate_id(conn);
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, win, screen->root, 0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY, screen->root_visual, 0, NULL);
    xcb_ewmh_set_supporting_wm_check(&ewmh, screen->root, win);
    xcb_ewmh_set_supporting_wm_check(&ewmh, win, win);
    xcb_ewmh_set_wm_name(&ewmh, win, 4, "ARWM");

    xcb_atom_t supported[] = {
        ewmh._NET_SUPPORTED, ewmh._NET_CLIENT_LIST, ewmh._NET_ACTIVE_WINDOW,
        ewmh._NET_WM_NAME, ewmh._NET_SUPPORTING_WM_CHECK, ewmh._NET_CURRENT_DESKTOP,
        ewmh._NET_NUMBER_OF_DESKTOPS, ewmh._NET_WM_STATE, ewmh._NET_WM_STATE_FULLSCREEN
    };
    xcb_ewmh_set_supported(&ewmh, 0, sizeof(supported)/sizeof(xcb_atom_t), supported);
}

void ewmh_update_client_list(void) {
    int count = 0;
    for (Client *c = clients; c; c = c->next) count++;
    if (count == 0) {
        xcb_ewmh_set_client_list(&ewmh, 0, 0, NULL);
        return;
    }
    xcb_window_t *wins = malloc(count * sizeof(xcb_window_t));
    int i = 0;
    for (Client *c = clients; c; c = c->next) wins[i++] = c->win;
    xcb_ewmh_set_client_list(&ewmh, 0, count, wins);
    free(wins);
}

void ewmh_update_active_window(xcb_window_t win) {
    xcb_ewmh_set_active_window(&ewmh, 0, win);
}
