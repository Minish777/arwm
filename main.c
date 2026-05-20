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

/* Error handler to detect existing window manager */
static int xerror_init(Display *d, XErrorEvent *e) { (void)d;
    if (e->error_code == BadAccess) {
        wm_detected = 1;
    }
    return 0;
}

/* Professional Error Handler: Logs but does NOT exit */
static int xerror(Display *d, XErrorEvent *e) {
    if (e->error_code == BadWindow) return 0;
    char err_buf[256];
    XGetErrorText(d, e->error_code, err_buf, sizeof(err_buf));
    fprintf(stderr, "ARWM [X11 Error]: %s (request_code=%d, minor_code=%d)\n",
            err_buf, e->request_code, e->minor_code);
    return 0;
}

static void setup_ewmh() {
    fprintf(stderr, "ARWM [Init]: Setting up EWMH atoms...\n");
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
    fprintf(stderr, "ARWM [Init]: EWMH setup complete.\n");
}

static void grab_keys() {
    fprintf(stderr, "ARWM [Init]: Grabbing keys and buttons...\n");
    KeyCode key;
    unsigned int modifiers = Mod4Mask;
    KeySym keys[] = { XK_Return, XK_d, XK_b, XK_q, XK_h, XK_v, XK_r, XK_f, XK_Tab, XK_space };

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        key = XKeysymToKeycode(dpy, keys[i]);
        if (key != 0) {
            XGrabKey(dpy, key, modifiers, root, True, GrabModeAsync, GrabModeAsync);
        }
    }

    XGrabButton(dpy, Button1, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, Button3, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
    fprintf(stderr, "ARWM [Init]: Grabs complete.\n");
}

static void signal_handler(int sig) {
    if (sig == SIGCHLD) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
        return;
    }
    fprintf(stderr, "ARWM [Signal]: Received signal %d, cleaning up...\n", sig);
    wm_cleanup();
    exit(0);
}

int main() {
    fprintf(stderr, "ARWM [Main]: Starting ARWM...\n");

    /* 1. Open Display */
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "ARWM [Fatal]: Cannot open display. Ensure X is running.\n");
        return 1;
    }
    fprintf(stderr, "ARWM [Init]: Display opened successfully.\n");

    root = DefaultRootWindow(dpy);

    /* 2. Set Initial Error Handler and Claim Root Window */
    XSetErrorHandler(xerror_init);
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);
    XSync(dpy, False);

    if (wm_detected) {
        fprintf(stderr, "ARWM [Fatal]: Another window manager is already running on this display.\n");
        XCloseDisplay(dpy);
        return 1;
    }
    fprintf(stderr, "ARWM [Init]: Root window claimed successfully.\n");

    /* 3. Set Professional Error Handler */
    XSetErrorHandler(xerror);

    /* 4. Global Signals */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, signal_handler);
    fprintf(stderr, "ARWM [Init]: Signal handlers installed.\n");

    /* 5. Configuration & EWMH setup */
    config_init();
    config_load();
    setup_ewmh();

    /* 6. Grabs */
    grab_keys();

    XSync(dpy, False);
    fprintf(stderr, "ARWM [Init]: Final sync complete. Entering event loop.\n");

    /* 7. Start Main Event Loop */
    wm_run();

    /* Should not be reached unless wm_run returns */
    fprintf(stderr, "ARWM [Main]: Event loop terminated unexpectedly.\n");
    wm_cleanup();
    XCloseDisplay(dpy);
    return 0;
}
