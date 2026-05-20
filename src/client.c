#include <stdlib.h>
#include "arwm.h"

Client *clients = NULL;

void client_manage(xcb_window_t win) {
    if (client_find(win)) return;
    xcb_get_window_attributes_cookie_t attr_cookie = xcb_get_window_attributes(conn, win);
    xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(conn, attr_cookie, NULL);
    if (!attr || attr->override_redirect) { free(attr); xcb_map_window(conn, win); return; }
    free(attr);

    Client *c = malloc(sizeof(Client));
    c->win = win; c->next = clients; clients = c;
    uint32_t mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, &mask);
    xcb_map_window(conn, win);
    layout_apply();
    client_focus(c);
    ewmh_update_client_list();
}

void client_unmanage(xcb_window_t win) {
    Client **curr = &clients;
    while (*curr) {
        if ((*curr)->win == win) {
            Client *tmp = *curr; *curr = (*curr)->next; free(tmp);
            layout_apply(); ewmh_update_client_list();
            return;
        }
        curr = &((*curr)->next);
    }
}

void client_focus(Client *c) {
    if (!c) return;
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, c->win, XCB_CURRENT_TIME);
    uint32_t bw[] = { cfg.border_width };
    xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_BORDER_WIDTH, bw);
    xcb_change_window_attributes(conn, c->win, XCB_CW_BORDER_PIXEL, (uint32_t[]){cfg.border_focus});
    for (Client *o = clients; o; o = o->next) {
        if (o != c) xcb_change_window_attributes(conn, o->win, XCB_CW_BORDER_PIXEL, (uint32_t[]){cfg.border_normal});
    }
    ewmh_update_active_window(c->win);
    xcb_flush(conn);
}

Client* client_find(xcb_window_t win) {
    for (Client *c = clients; c; c = c->next) if (c->win == win) return c;
    return NULL;
}
