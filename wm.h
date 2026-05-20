#ifndef WM_H
#define WM_H

#include <X11/Xlib.h>

typedef enum {
    SPLIT_NONE,
    SPLIT_HORIZONTAL,
    SPLIT_VERTICAL
} SplitType;

typedef struct Node {
    Window win;
    int x, y, w, h;
    SplitType split;
    int floating;
    struct Node *parent;
    struct Node *left;
    struct Node *right;

    GC gc;
} Node;

void wm_run();
void wm_handle_map_request(Window w);
void wm_handle_unmap_notify(Window w);
void wm_handle_destroy_notify(Window w);
void wm_handle_key_press(XKeyEvent *e);
void wm_handle_focus(Window w);
void wm_reload_config();
void spawn(const char *cmd);
void wm_ipc_init();
void wm_ipc_handle(int fd);
void wm_cleanup();

#endif
