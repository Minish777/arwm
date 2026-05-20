#include <stdlib.h>
#include "arwm.h"

void events_handle(xcb_generic_event_t *ev) {
    uint8_t type = ev->response_type & ~0x80;
    switch (type) {
        case XCB_MAP_REQUEST: {
            xcb_map_request_event_t *me = (xcb_map_request_event_t *)ev;
            client_manage(me->window);
            break;
        }
        case XCB_UNMAP_NOTIFY: {
            xcb_unmap_notify_event_t *ue = (xcb_unmap_notify_event_t *)ev;
            client_unmanage(ue->window);
            break;
        }
        case XCB_DESTROY_NOTIFY: {
            xcb_destroy_notify_event_t *de = (xcb_destroy_notify_event_t *)ev;
            client_unmanage(de->window);
            break;
        }
        case XCB_KEY_PRESS: {
            keybinds_handle((xcb_key_press_event_t *)ev);
            break;
        }
        case XCB_CONFIGURE_REQUEST: {
            xcb_configure_request_event_t *ce = (xcb_configure_request_event_t *)ev;
            uint16_t mask = ce->value_mask;
            uint32_t values[7];
            int i = 0;
            if (mask & XCB_CONFIG_WINDOW_X) values[i++] = ce->x;
            if (mask & XCB_CONFIG_WINDOW_Y) values[i++] = ce->y;
            if (mask & XCB_CONFIG_WINDOW_WIDTH) values[i++] = ce->width;
            if (mask & XCB_CONFIG_WINDOW_HEIGHT) values[i++] = ce->height;
            if (mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) values[i++] = ce->border_width;
            if (mask & XCB_CONFIG_WINDOW_SIBLING) values[i++] = ce->sibling;
            if (mask & XCB_CONFIG_WINDOW_STACK_MODE) values[i++] = ce->stack_mode;
            xcb_configure_window(conn, ce->window, mask, values);
            xcb_flush(conn);
            break;
        }
        case XCB_ENTER_NOTIFY: {
            xcb_enter_notify_event_t *ee = (xcb_enter_notify_event_t *)ev;
            Client *c = client_find(ee->event);
            if (c) client_focus(c);
            break;
        }
    }
}
