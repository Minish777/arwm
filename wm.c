#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sched.h>
#include <poll.h>
#include <errno.h>

#include "wm.h"
#include "config.h"

#ifdef DEBUG
#define DBG(...) fprintf(stderr, "ARWM: " __VA_ARGS__)
#else
#define DBG(...)
#endif

extern Display *dpy;
extern Window root;

static Node *tree_root = NULL;
static SplitType next_split = SPLIT_VERTICAL;
static Window focused_win = None;

// Canvas management
#define MAX_WINDOWS 64
static struct pollfd fds[MAX_WINDOWS + 1];
static Node* fd_to_node[MAX_WINDOWS + 1];
static int nfds = 1;

/* ---------- CANVAS PROTOCOL ---------- */
typedef struct {
    unsigned char type;
    union {
        struct { unsigned int color; } clear;
        struct { short x, y, w, h; unsigned int color; } rect;
        struct { short x, y; unsigned int color; } point;
    } data;
} CanvasCmd;

/* ---------- UTILS ---------- */

static Node* create_node(Window w) {
    Node *n = calloc(1, sizeof(Node));
    if (!n) return NULL;
    n->win = w;
    n->split = SPLIT_NONE;
    n->canvas_fd = -1;
    n->gc = XCreateGC(dpy, w, 0, NULL);
    return n;
}

static void apply_node_geometry(Node *n) {
    if (!n) return;
    if (n->win != None) {
        int bw = cfg_border_width;
        XSetWindowBorderWidth(dpy, n->win, bw);
        XSetWindowBorder(dpy, n->win, (n->win == focused_win) ? cfg_border_active : cfg_border_inactive);
        XMoveResizeWindow(dpy, n->win, n->x + cfg_gap, n->y + cfg_gap,
            (n->w - cfg_gap * 2 - bw * 2 > 0) ? n->w - cfg_gap * 2 - bw * 2 : 1,
            (n->h - cfg_gap * 2 - bw * 2 > 0) ? n->h - cfg_gap * 2 - bw * 2 : 1);
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
    XSync(dpy, False);
}

static Node* find_node_by_window(Node *n, Window w) {
    if (!n) return NULL;
    if (n->win == w) return n;
    Node *res = find_node_by_window(n->left, w);
    if (res) return res;
    return find_node_by_window(n->right, w);
}

static void handle_canvas_input(Node *n) {
    CanvasCmd cmd;
    while (read(n->canvas_fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd.type) {
            case 0:
                XSetForeground(dpy, n->gc, cmd.data.clear.color);
                XFillRectangle(dpy, n->win, n->gc, 0, 0, n->w, n->h);
                break;
            case 1:
                XSetForeground(dpy, n->gc, cmd.data.rect.color);
                XFillRectangle(dpy, n->win, n->gc, cmd.data.rect.x, cmd.data.rect.y, cmd.data.rect.w, cmd.data.rect.h);
                break;
            case 2:
                XSetForeground(dpy, n->gc, cmd.data.point.color);
                XDrawPoint(dpy, n->win, n->gc, cmd.data.point.x, cmd.data.point.y);
                break;
        }
    }
    XFlush(dpy);
}

static void remove_window(Window w) {
    Node *n = find_node_by_window(tree_root, w);
    if (!n) return;
    if (n->canvas_fd != -1) {
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == n->canvas_fd) {
                fds[i] = fds[nfds - 1];
                fd_to_node[i] = fd_to_node[nfds - 1];
                nfds--;
                break;
            }
        }
        close(n->canvas_fd);
        char path[256];
        snprintf(path, sizeof(path), "/tmp/arwm_canvas_%lu", w);
        unlink(path);
    }
    XFreeGC(dpy, n->gc);
    if (focused_win == w) focused_win = None;
    Node *p = n->parent;
    if (!p) {
        free(tree_root); tree_root = NULL;
    } else {
        Node *sibling = (p->left == n) ? p->right : p->left;
        Node *gp = p->parent;
        if (!gp) { tree_root = sibling; sibling->parent = NULL; }
        else {
            if (gp->left == p) gp->left = sibling;
            else gp->right = sibling;
            sibling->parent = gp;
        }
        free(n); free(p);
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

void wm_handle_focus(Window w) {
    if (w == None || w == root) return;
    if (!find_node_by_window(tree_root, w)) return;
    Window old_focused = focused_win;
    focused_win = w;
    if (old_focused != None) XSetWindowBorder(dpy, old_focused, cfg_border_inactive);
    XSetWindowBorder(dpy, focused_win, cfg_border_active);
    XRaiseWindow(dpy, focused_win);
    XSetInputFocus(dpy, focused_win, RevertToPointerRoot, CurrentTime);
}

void wm_handle_map_request(Window w) {
    XWindowAttributes wa;
    XGetWindowAttributes(dpy, w, &wa);
    if (wa.override_redirect) return;
    XSelectInput(dpy, w, StructureNotifyMask | EnterWindowMask | FocusChangeMask | ExposureMask);
    XMapWindow(dpy, w);

    Node *n = NULL;
    if (!tree_root) {
        tree_root = n = create_node(w);
    } else {
        Node *target = find_insertion_point(tree_root);
        if (target) {
            Node *old_leaf = create_node(target->win);
            Node *new_leaf = n = create_node(w);
            target->win = None; target->split = next_split;
            target->left = old_leaf; target->right = new_leaf;
            old_leaf->parent = target; new_leaf->parent = target;
        }
    }

    // Check if there's a pending canvas for this window
    char pipe_path[256];
    snprintf(pipe_path, sizeof(pipe_path), "/tmp/arwm_canvas_%lu", w);
    if (n && access(pipe_path, F_OK) == 0) {
        int fd = open(pipe_path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            n->canvas_fd = fd;
            fds[nfds].fd = fd;
            fds[nfds].events = POLLIN;
            fd_to_node[nfds] = n;
            nfds++;
        }
    }

    wm_handle_focus(w);
    refresh_tree();
}

void wm_handle_unmap_notify(Window w) { remove_window(w); }
void wm_handle_destroy_notify(Window w) { remove_window(w); }
void wm_reload_config() { config_load(); refresh_tree(); }

void wm_handle_key_press(XKeyEvent *e) {
    KeySym keysym = XLookupKeysym(e, 0);
    if (keysym == XK_Return) spawn("alacritty");
    else if (keysym == XK_d) spawn("dmenu_run");
    else if (keysym == XK_b) spawn("firefox");
    else if (keysym == XK_h) next_split = SPLIT_VERTICAL;
    else if (keysym == XK_v) next_split = SPLIT_HORIZONTAL;
    else if (keysym == XK_r) wm_reload_config();
    else if (keysym == XK_q) exit(0);
}

void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        if (fork() == 0) {
            execl("/bin/sh", "sh", "-c", cmd, NULL);
            exit(1);
        }
        exit(0);
    }
    wait(NULL);
}

void spawn_canvas(const char *cmd) {
    if (fork() == 0) {
        unshare(CLONE_NEWNS);
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(0);
    }
}

void wm_run() {
    int x11_fd = ConnectionNumber(dpy);
    fds[0].fd = x11_fd;
    fds[0].events = POLLIN;
    XEvent ev;
    while (1) {
        while (XPending(dpy)) {
            XNextEvent(dpy, &ev);
            switch (ev.type) {
                case MapRequest: wm_handle_map_request(ev.xmaprequest.window); break;
                case UnmapNotify: wm_handle_unmap_notify(ev.xunmap.window); break;
                case DestroyNotify: wm_handle_destroy_notify(ev.xdestroywindow.window); break;
                case KeyPress: wm_handle_key_press(&ev.xkey); break;
                case EnterNotify: wm_handle_focus(ev.xcrossing.window); break;
                case Expose: refresh_tree(); break;
            }
        }
        if (poll(fds, nfds, -1) > 0) {
            if (fds[0].revents & POLLIN) continue; // X11 event ready
            for (int i = 1; i < nfds; i++) {
                if (fds[i].revents & POLLIN) {
                    handle_canvas_input(fd_to_node[i]);
                }
            }
        }
    }
}
