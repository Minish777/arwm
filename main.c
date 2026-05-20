#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "wm.h"
#include "config.h"

Display *dpy;
Window root;
int wm_detected = 0;

/* Atoms for EWMH */
Atom net_supported, net_client_list, net_active_window, net_wm_name, utf8_string;
Atom net_wm_state, net_wm_state_fullscreen, net_wm_window_type, net_wm_window_type_dialog;
Atom net_number_of_desktops, net_current_desktop;

/* Robust Error Handling: detect existing WM or BadAccess during grabs */
int xerror_init(Display *d, XErrorEvent *e) {
    if (e->error_code == BadAccess) {
        wm_detected = 1;
    }
    return 0;
}

/* Fault-Tolerant Error Handling: prints errors but does NOT exit */
int xerror(Display *d, XErrorEvent *e) {
    if (e->error_code == BadWindow) return 0;
    char err_buf[256];
    XGetErrorText(d, e->error_code, err_buf, sizeof(err_buf));
    fprintf(stderr, "ARWM: X Error: %s (error_code=%d, request_code=%d, minor_code=%d)\n",
            err_buf, e->error_code, e->request_code, e->minor_code);
    return 0;
}

static void setup_ewmh() {
    net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
    net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
    net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    net_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    net_wm_window_type_dialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    net_number_of_desktops = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
    net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);

    Atom supported[] = {
        net_supported, net_client_list, net_active_window, net_wm_name,
        net_wm_state, net_wm_state_fullscreen, net_wm_window_type, net_wm_window_type_dialog,
        net_number_of_desktops, net_current_desktop
    };
    XChangeProperty(dpy, root, net_supported, XA_ATOM, 32, PropModeReplace, (unsigned char*)supported, sizeof(supported)/sizeof(Atom));

    unsigned long one = 1, zero = 0;
    XChangeProperty(dpy, root, net_number_of_desktops, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&one, 1);
    XChangeProperty(dpy, root, net_current_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&zero, 1);

    XStoreName(dpy, root, "ARWM");
    XChangeProperty(dpy, root, net_wm_name, utf8_string, 8, PropModeReplace, (unsigned char*)"ARWM", 4);
}

static void grab_keys() {
    KeyCode key;
    unsigned int modifiers = Mod4Mask;
    KeySym keys[] = { XK_Return, XK_d, XK_b, XK_q, XK_h, XK_v, XK_r, XK_f, XK_Tab, XK_space };

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        key = XKeysymToKeycode(dpy, keys[i]);
        if (key == 0) continue;

        /* Validation: Ensure request doesn't crash us */
        XGrabKey(dpy, key, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    }
}

void signal_handler(int sig) {
    if (sig == SIGCHLD) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
        return;
    }
    wm_cleanup();
    exit(0);
}

int main() {
    /* Display Check */
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "ARWM: FATAL: Cannot open display\n");
        return 1;
    }

    root = DefaultRootWindow(dpy);

    /* 1. Robust Initialization Sequence: Claim root window first */
    XSetErrorHandler(xerror_init);
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);
    XSync(dpy, False);

    if (wm_detected) {
        fprintf(stderr, "ARWM: FATAL: Another window manager is already running on this display.\n");
        XCloseDisplay(dpy);
        return 1;
    }

    /* 2. Switch to Fault-Tolerant Error Handling */
    XSetErrorHandler(xerror);

    /* 3. Global Signals */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, signal_handler);

    /* 4. Configuration & EWMH */
    config_init();
    config_load();
    setup_ewmh();

    /* 5. Validation: Grabs occur only after root is claimed */
    grab_keys();

    XGrabButton(dpy, Button1, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, Button3, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XSync(dpy, False);

    /* 6. Event Loop (inside wm_run uses XPending) */
    wm_run();

    wm_cleanup();
    XCloseDisplay(dpy);
    return 0;
}
