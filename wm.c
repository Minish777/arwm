#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <errno.h>

#include "wm.h"
#include "config.h"

#define MAX_WINDOWS 128

extern Display *dpy;
extern Window root;
extern Atom net_client_list, net_active_window, net_wm_window_type, net_wm_window_type_dialog;

static Node *tree_root = NULL;
static SplitType next_split = SPLIT_VERTICAL;
static Window focused_win = None;
static int ipc_sock = -1;

static int mouse_start_x, mouse_start_y;
static int win_start_x, win_start_y, win_start_w, win_start_h;

/* ---------- UTILS ---------- */

static Node* create_node(Window w) {
    Node *n = calloc(1, sizeof(Node));
    if (!n) return NULL;
    n->win = w;
    n->split = SPLIT_NONE;
    if (w != None) n->gc = XCreateGC(dpy, w, 0, NULL);
    return n;
}

static void get_all_windows(Node *n, Window *wins, int *count) {
    if (!n || *count >= MAX_WINDOWS) return;
    if (n->win != None) wins[(*count)++] = n->win;
    get_all_windows(n->left, wins, count);
    get_all_windows(n->right, wins, count);
}

static void update_client_list() {
    Window wins[MAX_WINDOWS];
    int count = 0;
    get_all_windows(tree_root, wins, &count);
    XChangeProperty(dpy, root, net_client_list, XA_WINDOW, 32, PropModeReplace, (unsigned char*)wins, count);
}

static void apply_node_geometry(Node *n) {
    if (!n) return;
    if (n->win != None && !n->floating) {
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
    update_tree_geometries(tree_root, 0, 0, DisplayWidth(dpy, DefaultScreen(dpy)), DisplayHeight(dpy, DefaultScreen(dpy)));
    apply_node_geometry(tree_root);
    XSync(dpy, False);
}

static Node* find_node_by_window(Node *n, Window w) {
    if (!n) return NULL;
    if (n->win == w) return n;
    Node *res = find_node_by_window(n->left, w);
    return res ? res : find_node_by_window(n->right, w);
}

static void remove_window(Window w) {
    Node *n = find_node_by_window(tree_root, w);
    if (!n) return;
    if (n->gc) XFreeGC(dpy, n->gc);
    if (focused_win == w) focused_win = None;
    Node *p = n->parent;
    if (!p) { free(tree_root); tree_root = NULL; }
    else {
        Node *sibling = (p->left == n) ? p->right : p->left;
        Node *gp = p->parent;
        if (!gp) { tree_root = sibling; sibling->parent = NULL; }
        else {
            if (gp->left == p) gp->left = sibling; else gp->right = sibling;
            sibling->parent = gp;
        }
        free(n); free(p);
    }
    refresh_tree();
    update_client_list();
}

static Node* find_insertion_point(Node *n) {
    if (!n) return NULL;
    if (n->win != None && !n->floating) return n;
    if (n->right) { Node *res = find_insertion_point(n->right); if (res) return res; }
    return find_insertion_point(n->left);
}

void wm_handle_focus(Window w) {
    if (w == None || w == root) return;
    if (!find_node_by_window(tree_root, w)) return;
    if (focused_win != None) XSetWindowBorder(dpy, focused_win, cfg_border_inactive);
    focused_win = w;
    XSetWindowBorder(dpy, focused_win, cfg_border_active);
    XRaiseWindow(dpy, focused_win);
    XSetInputFocus(dpy, focused_win, RevertToPointerRoot, CurrentTime);
    XChangeProperty(dpy, root, net_active_window, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&focused_win, 1);
}

void wm_focus_next() {
    Window wins[MAX_WINDOWS]; int count = 0;
    get_all_windows(tree_root, wins, &count);
    if (count <= 1) return;
    for (int i = 0; i < count; i++) if (wins[i] == focused_win) { wm_handle_focus(wins[(i + 1) % count]); return; }
    wm_handle_focus(wins[0]);
}

void wm_handle_map_request(Window w) {
    XWindowAttributes wa; XGetWindowAttributes(dpy, w, &wa);
    if (wa.override_redirect) return;
    int floating = 0;
    Atom actual_type; int actual_format; unsigned long nitems, bytes_after; unsigned char *prop = NULL;
    if (XGetWindowProperty(dpy, w, net_wm_window_type, 0, sizeof(Atom), False, XA_ATOM, &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        if (*(Atom*)prop == net_wm_window_type_dialog) floating = 1;
        XFree(prop);
    }
    XSelectInput(dpy, w, StructureNotifyMask | EnterWindowMask | FocusChangeMask);
    XMapWindow(dpy, w);
    Node *n = NULL;
    if (!tree_root) tree_root = n = create_node(w);
    else {
        Node *target = find_insertion_point(tree_root);
        if (target) {
            Node *old_leaf = create_node(target->win); old_leaf->floating = target->floating;
            Node *new_leaf = n = create_node(w);
            target->win = None; target->split = next_split; target->left = old_leaf; target->right = new_leaf;
            old_leaf->parent = target; new_leaf->parent = target;
        } else {
            Node *old_root = tree_root; tree_root = create_node(None);
            tree_root->split = next_split; tree_root->left = old_root; tree_root->right = n = create_node(w);
            old_root->parent = tree_root; tree_root->right->parent = tree_root;
        }
    }
    if (n) n->floating = floating;
    wm_handle_focus(w); refresh_tree(); update_client_list();
}

void wm_handle_unmap_notify(Window w) { remove_window(w); }
void wm_handle_destroy_notify(Window w) { remove_window(w); }
void wm_reload_config() { config_load(); refresh_tree(); }

void wm_handle_key_press(XKeyEvent *e) {
    KeySym k = XLookupKeysym(e, 0);
    if (k == XK_Return) spawn("alacritty || xterm");
    else if (k == XK_d) spawn("dmenu_run");
    else if (k == XK_b) spawn("firefox");
    else if (k == XK_f) spawn("thunar || pcmanfm || nautilus");
    else if (k == XK_Tab) wm_focus_next();
    else if (k == XK_h) next_split = SPLIT_VERTICAL;
    else if (k == XK_v) next_split = SPLIT_HORIZONTAL;
    else if (k == XK_r) wm_reload_config();
    else if (k == XK_space) { Node *n = find_node_by_window(tree_root, focused_win); if (n) { n->floating = !n->floating; refresh_tree(); } }
    else if (k == XK_q) exit(0);
}

void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        if (fork() == 0) { execl("/bin/sh", "sh", "-c", cmd, NULL); exit(1); }
        exit(0);
    }
}

void wm_ipc_init() {
    struct sockaddr_un addr; ipc_sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (ipc_sock == -1) return;
    memset(&addr, 0, sizeof(addr)); addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/arwm.sock");
    unlink(addr.sun_path);
    if (bind(ipc_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) return;
    listen(ipc_sock, 5);
}

void wm_ipc_handle(int fd) {
    int client = accept(fd, NULL, NULL); if (client == -1) return;
    char buf[256]; ssize_t r = read(client, buf, sizeof(buf) - 1);
    if (r > 0) {
        buf[r] = '\0';
        if (strncmp(buf, "quit", 4) == 0) exit(0);
        else if (strncmp(buf, "reload", 6) == 0) wm_reload_config();
        else if (strncmp(buf, "spawn ", 6) == 0) spawn(buf + 6);
        else if (strncmp(buf, "focus_next", 10) == 0) wm_focus_next();
    }
    close(client);
}

void wm_cleanup() { unlink("/tmp/arwm.sock"); if (ipc_sock != -1) close(ipc_sock); }

static void autostart() {
    char path[512]; snprintf(path, sizeof(path), "%s/.config/arwm/autostart.sh", getenv("HOME"));
    if (access(path, X_OK) == 0) spawn(path);
}

void wm_run() {
    wm_ipc_init(); autostart();
    struct pollfd pfd[2]; pfd[0].fd = ConnectionNumber(dpy); pfd[0].events = POLLIN;
    pfd[1].fd = ipc_sock; pfd[1].events = POLLIN;
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
                case ButtonPress:
                    if (ev.xbutton.subwindow != None) {
                        wm_handle_focus(ev.xbutton.subwindow);
                        XWindowAttributes wa; XGetWindowAttributes(dpy, ev.xbutton.subwindow, &wa);
                        mouse_start_x = ev.xbutton.x_root; mouse_start_y = ev.xbutton.y_root;
                        win_start_x = wa.x; win_start_y = wa.y; win_start_w = wa.width; win_start_h = wa.height;
                    }
                    break;
                case MotionNotify:
                    if (focused_win != None && ev.xmotion.state & Mod4Mask) {
                        int dx = ev.xmotion.x_root - mouse_start_x, dy = ev.xmotion.y_root - mouse_start_y;
                        if (ev.xmotion.state & Button1Mask) XMoveWindow(dpy, focused_win, win_start_x + dx, win_start_y + dy);
                        else if (ev.xmotion.state & Button3Mask) XResizeWindow(dpy, focused_win, (win_start_w+dx>10)?win_start_w+dx:10, (win_start_h+dy>10)?win_start_h+dy:10);
                    }
                    break;
            }
        }
        if (poll(pfd, 2, 10) > 0) if (pfd[1].revents & POLLIN) wm_ipc_handle(ipc_sock);
    }
}
