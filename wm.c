#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#include "wm.h"
#include "config.h"

extern Display *dpy;
extern Window root;

static Node *tree_root = NULL;
static SplitType next_split = SPLIT_VERTICAL;

/* ---------- UTILS ---------- */

static Node* create_node(Window w) {
    Node *n = calloc(1, sizeof(Node));
    n->win = w;
    n->split = SPLIT_NONE;
    return n;
}

static void apply_node_geometry(Node *n) {
    if (!n) return;
    if (n->win != None) {
        XMoveResizeWindow(dpy, n->win,
            n->x + cfg_gap,
            n->y + cfg_gap,
            n->w - cfg_gap * 2,
            n->h - cfg_gap * 2);
    }
    apply_node_geometry(n->left);
    apply_node_geometry(n->right);
}

static void update_tree_geometries(Node *n, int x, int y, int w, int h) {
    if (!n) return;
    n->x = x; n->y = y; n->w = w; n->h = h;

    if (n->split == SPLIT_VERTICAL) {
        int lw = w * cfg_master_ratio;
        update_tree_geometries(n->left, x, y, lw, h);
        update_tree_geometries(n->right, x + lw, y, w - lw, h);
    } else if (n->split == SPLIT_HORIZONTAL) {
        int lh = h * cfg_master_ratio;
        update_tree_geometries(n->left, x, y, w, lh);
        update_tree_geometries(n->right, x, y + lh, w, h - lh);
    }
}

static void refresh_tree() {
    if (!tree_root) return;
    int sw = DisplayWidth(dpy, DefaultScreen(dpy));
    int sh = DisplayHeight(dpy, DefaultScreen(dpy));
    update_tree_geometries(tree_root, 0, 0, sw, sh);
    apply_node_geometry(tree_root);
}

static Node* find_node_by_window(Node *n, Window w) {
    if (!n) return NULL;
    if (n->win == w) return n;
    Node *res = find_node_by_window(n->left, w);
    if (res) return res;
    return find_node_by_window(n->right, w);
}

static void remove_window(Window w) {
    Node *n = find_node_by_window(tree_root, w);
    if (!n) return;

    Node *p = n->parent;
    if (!p) {
        free(tree_root);
        tree_root = NULL;
    } else {
        Node *sibling = (p->left == n) ? p->right : p->left;
        Node *gp = p->parent;

        if (!gp) {
            tree_root = sibling;
            sibling->parent = NULL;
        } else {
            if (gp->left == p) gp->left = sibling;
            else gp->right = sibling;
            sibling->parent = gp;
        }
        free(n);
        free(p);
    }
    refresh_tree();
}

static Node* find_insertion_point(Node *n) {
    if (!n) return NULL;
    if (n->win != None) return n;
    if (n->right) return find_insertion_point(n->right);
    return find_insertion_point(n->left);
}

/* ---------- HANDLERS ---------- */

void wm_handle_map_request(Window w) {
    XSelectInput(dpy, w, StructureNotifyMask | EnterWindowMask);
    XMapWindow(dpy, w);

    if (!tree_root) {
        tree_root = create_node(w);
    } else {
        Node *target = find_insertion_point(tree_root);
        if (target) {
            Node *old_leaf = create_node(target->win);
            Node *new_leaf = create_node(w);

            target->win = None;
            target->split = next_split;
            target->left = old_leaf;
            target->right = new_leaf;
            old_leaf->parent = target;
            new_leaf->parent = target;
        }
    }
    refresh_tree();
}

void wm_handle_unmap_notify(Window w) {
    remove_window(w);
}

void wm_handle_destroy_notify(Window w) {
    remove_window(w);
}

void wm_handle_key_press(XKeyEvent *e) {
    KeySym keysym = XLookupKeysym(e, 0);
    if (keysym == XK_Return) {
        spawn("alacritty");
    } else if (keysym == XK_d) {
        spawn("dmenu_run");
    } else if (keysym == XK_b) {
        spawn("firefox");
    } else if (keysym == XK_h) {
        next_split = SPLIT_VERTICAL;
    } else if (keysym == XK_v) {
        next_split = SPLIT_HORIZONTAL;
    } else if (keysym == XK_q) {
        exit(0);
    }
}

void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        if (fork() == 0) {
            execlp(cmd, cmd, NULL);
            if (strcmp(cmd, "alacritty") == 0) {
                execlp("xterm", "xterm", NULL);
            }
            exit(1);
        }
        exit(0);
    }
    wait(NULL);
}

void wm_run() {
    spawn("alacritty");
    XEvent ev;
    while (1) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
            case MapRequest:
                wm_handle_map_request(ev.xmaprequest.window);
                break;
            case UnmapNotify:
                wm_handle_unmap_notify(ev.xunmap.window);
                break;
            case DestroyNotify:
                wm_handle_destroy_notify(ev.xdestroywindow.window);
                break;
            case KeyPress:
                wm_handle_key_press(&ev.xkey);
                break;
        }
    }
}
