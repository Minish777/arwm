#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>

#include "wm.h"
#include "config.h"

Display *dpy;
Window root;

int xerror(Display *d, XErrorEvent *e) {
    return 0;
}

/* ---------- WM NAME (fastfetch fix) ---------- */
static void set_wm_name() {

    XStoreName(dpy, root, "ARWM");

    Atom net = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8 = XInternAtom(dpy, "UTF8_STRING", False);

    XChangeProperty(dpy, root, net, utf8, 8,
        PropModeReplace,
        (unsigned char*)"ARWM", 4);
}

/* ---------- GRAB KEYS (CRITICAL FIX) ---------- */
static void grab_keys() {

    unsigned int mod = Mod4Mask;

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Return),
        mod, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_d),
        mod, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_b),
        mod, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_q),
        mod, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_h),
        mod, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_v),
        mod, root, True, GrabModeAsync, GrabModeAsync);
}

int main() {

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "ARWM: cannot open display\n");
        return 1;
    }

    XSetErrorHandler(xerror);

    root = DefaultRootWindow(dpy);

    XSelectInput(dpy, root,
        SubstructureRedirectMask |
        SubstructureNotifyMask |
        KeyPressMask |
        PointerMotionMask
    );

    XSync(dpy, False);

    config_init();
    config_load();

    set_wm_name();
    grab_keys();

    wm_run();

    return 0;
}