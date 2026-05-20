#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>

#include "wm.h"
#include "config.h"

Display *dpy;
Window root;

int xerror(Display *d, XErrorEvent *e) {
    if (e->error_code == BadWindow) return 0;
    fprintf(stderr, "ARWM: X error: error_code=%d, request_code=%d, minor_code=%d\n",
            e->error_code, e->request_code, e->minor_code);
    return 0;
}

static void set_wm_name() {
    XStoreName(dpy, root, "ARWM");
    Atom net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
    XChangeProperty(dpy, root, net_wm_name, utf8_string, 8,
                    PropModeReplace, (unsigned char*)"ARWM", 4);
}

static void grab_keys() {
    KeyCode key;
    unsigned int modifiers = Mod4Mask;
    KeySym keys[] = { XK_Return, XK_d, XK_b, XK_q, XK_h, XK_v };

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        key = XKeysymToKeycode(dpy, keys[i]);
        XGrabKey(dpy, key, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    }
}

int main() {
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "ARWM: Cannot open display\n");
        return 1;
    }

    XSetErrorHandler(xerror);
    root = DefaultRootWindow(dpy);

    config_init();
    config_load();
    set_wm_name();
    grab_keys();

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);
    XSync(dpy, False);

    wm_run();

    XCloseDisplay(dpy);
    return 0;
}
