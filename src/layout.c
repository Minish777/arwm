#include <stdlib.h>
#include <stdbool.h>
#include "arwm.h"

static void dwindle(int x, int y, int w, int h, Client *c) {
    if (!c) return;
    if (!c->next) {
        uint32_t vals[] = { (uint32_t)(x + cfg.gap_outer), (uint32_t)(y + cfg.gap_outer),
                            (uint32_t)(w - 2*cfg.gap_outer), (uint32_t)(h - 2*cfg.gap_outer) };
        xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, vals);
        return;
    }

    bool vertical = (w > h);
    int nw = vertical ? w / 2 : w;
    int nh = vertical ? h : h / 2;

    uint32_t vals[4];
    vals[0] = x + cfg.gap_outer;
    vals[1] = y + cfg.gap_outer;
    if (vertical) {
        vals[2] = nw - cfg.gap_inner/2 - cfg.gap_outer;
        vals[3] = h - 2*cfg.gap_outer;
    } else {
        vals[2] = w - 2*cfg.gap_outer;
        vals[3] = nh - cfg.gap_inner/2 - cfg.gap_outer;
    }

    xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, vals);

    if (vertical) dwindle(x + nw + cfg.gap_inner/2, y, w - nw - cfg.gap_inner/2, h, c->next);
    else dwindle(x, y + nh + cfg.gap_inner/2, w, h - nh - cfg.gap_inner/2, c->next);
}

void layout_apply(void) {
    for (Monitor *m = monitors; m; m = m->next) {
        layout_tile_bsp(m);
    }
    xcb_flush(conn);
}

void layout_tile_bsp(Monitor *m) {
    if (!clients) return;
    dwindle(m->x, m->y, m->w, m->h, clients);
}
