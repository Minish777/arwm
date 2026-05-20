#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xcb.h>
#include <stdbool.h>

typedef struct Client {
    xcb_window_t win;
    int x, y, w, h;
    bool is_floating;
    bool is_fullscreen;
    struct Client *next;
} Client;

extern Client *clients;

void client_manage(xcb_window_t win);
void client_unmanage(xcb_window_t win);
void client_focus(Client *c);
Client* client_find(xcb_window_t win);

#endif
