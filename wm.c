#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "config.h"

extern Display *dpy;
extern Window root;

/* ---------- NODE ---------- */
typedef struct Node {
    Window w;
    int x, y, ww, hh;
    int leaf;

    struct Node *l;
    struct Node *r;

} Node;

Node *tree = NULL;

/* ---------- SPAWN ---------- */
void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh","sh","-c",cmd,NULL);
        exit(0);
    }
}

/* ---------- LEAF ---------- */
Node* leaf(Window w, int x, int y, int ww, int hh) {
    Node *n = malloc(sizeof(Node));
    n->w = w;
    n->x = x;
    n->y = y;
    n->ww = ww;
    n->hh = hh;
    n->leaf = 1;
    n->l = n->r = NULL;
    return n;
}

/* ---------- SPLIT ---------- */
Node* split(Node *n, Window w, int vertical) {

    if (!n)
        return leaf(w, 0, 0, 1280, 720);

    if (n->leaf) {

        Window old = n->w;

        Node *a = leaf(old, n->x, n->y, n->ww, n->hh);
        Node *b = leaf(w, n->x, n->y, n->ww, n->hh);

        n->leaf = 0;

        if (vertical) {
            a->ww = n->ww * cfg_master_ratio;
            b->x = n->x + a->ww;
            b->ww = n->ww - a->ww;
        } else {
            a->hh = n->hh * cfg_master_ratio;
            b->y = n->y + a->hh;
            b->hh = n->hh - a->hh;
        }

        n->l = a;
        n->r = b;

        return n;
    }

    if (n->l) n->l = split(n->l, w, vertical);
    else n->r = split(n->r, w, vertical);

    return n;
}

/* ---------- APPLY ---------- */
void apply(Node *n) {

    if (!n) return;

    if (n->leaf) {

        XMoveResizeWindow(dpy, n->w,
            n->x + cfg_gap,
            n->y + cfg_gap,
            n->ww - cfg_gap * 2,
            n->hh - cfg_gap * 2
        );

        return;
    }

    apply(n->l);
    apply(n->r);
}

/* ---------- MAP WINDOW ---------- */
void map_window(Window w) {

    XMapWindow(dpy, w);

    if (!tree)
        tree = leaf(w, 0, 0, 1280, 720);
    else
        tree = split(tree, w, 1);

    apply(tree);
}

/* ---------- KEY HANDLER ---------- */
void handle_key(XKeyEvent *e) {

    KeySym k = XLookupKeysym(e, 0);

    if (!(e->state & Mod4Mask))
        return;

    switch (k) {

        case XK_Return:
            spawn("alacritty");
            break;

        case XK_d:
            spawn("dmenu_run");
            break;

        case XK_b:
            spawn("firefox");
            break;

        case XK_h:
            tree = split(tree, tree->w, 1);
            apply(tree);
            break;

        case XK_v:
            tree = split(tree, tree->w, 0);
            apply(tree);
            break;

        case XK_q:
            exit(0);
    }
}

/* ---------- LOOP ---------- */
void wm_run() {

    /* 🔥 FIX: start usable session immediately */
    spawn("alacritty &");

    XEvent e;

    while (1) {

        XNextEvent(dpy, &e);

        switch (e.type) {

            case MapRequest:
                map_window(e.xmaprequest.window);
                break;

            case KeyPress:
                handle_key(&e.xkey);
                break;
        }
    }
}